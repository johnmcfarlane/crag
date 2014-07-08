//
//  Engine.cpp
//  crag
//
//  Created by John on 12/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Engine.h"

#include "Debug.h"
#include "IndexedVboResource.h"
#include "Messages.h"
#include "Program.h"
#include "RegisterResources.h"
#include "Scene.h"
#include "SetCameraEvent.h"
#include "SetOriginEvent.h"

#include "form/Engine.h"

#include "sim/Engine.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/ResourceManager.h"
#include "core/Statistics.h"

#if defined(NDEBUG)
#define INIT_CAP(CAP,ENABLED) { CAP,ENABLED }
#else
#define INIT_CAP(CAP,ENABLED) { CAP,ENABLED,#CAP }

//#define CRAG_DEBUG_SHADOW_VOLUMES
#endif

using core::Time;
using namespace gfx;

CONFIG_DEFINE(depth_func, int, GL_LESS);
CONFIG_DEFINE (shadows_enabled, bool, true);
CONFIG_DECLARE(profile_mode, bool);
CONFIG_DECLARE(camera_near, float);

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// File-local Variables
	
	CONFIG_DEFINE (global_ambient, Color4f, Color4f(.01f,.01f,.02f));
	CONFIG_DEFINE (default_refresh_rate, int, 50);
	CONFIG_DEFINE (swap_interval, int, 1);

	CONFIG_DEFINE (init_culling, bool, true);
	
	CONFIG_DEFINE (capture_enable, bool, false);
	CONFIG_DEFINE (capture_skip, int, 0);
	
#if defined(CRAG_USE_GLES)
	CONFIG_DEFINE (max_foreground_depth, float, 0.99999f);
#else
	CONFIG_DEFINE (max_foreground_depth, float, 0.9999999f);
#endif

	// TODO
	//CONFIG_DEFINE (record_enable, bool, false);
	//CONFIG_DEFINE (record_playback, bool, false);
	//CONFIG_DEFINE (capture, bool, false);
	//CONFIG_DEFINE (record_playback_skip, int, 1);

	STAT (frame_duration, double, .15f);
	STAT (fps, float, .0f);
	STAT_DEFAULT (pos, sim::Vector3, .3f, sim::Vector3::Zero());
	STAT_DEFAULT (z_range, sim::Vector2, .78f, sim::Vector2::Zero());
	
	////////////////////////////////////////////////////////////////////////////////
	// File-local functions
	
	bool InitGlew()
	{
#if defined(GLEW_STATIC)
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK)
		{
			ERROR_MESSAGE("GLEW Error: %s", glewGetErrorString(glew_err));
			return false;
		}

		DEBUG_MESSAGE("GLEW Version: %s", glewGetString(GLEW_VERSION));
		
#if ! defined(__APPLE__)
		if (! GLEW_VERSION_1_5)
		{
			ERROR_MESSAGE("Error: Crag requires OpenGL 1.5 or greater.");
			return false;
		}
#endif
		
#endif
		
		return true;
	}
	
	void setDepthRange(float near, float far)
	{
#if defined(WIN32)
		glDepthRange(near, far);
#else
		glDepthRangef(near, far);
#endif
	}

	// Given the list of objects to render, calculates suitable near/far z values.
	RenderRange CalculateDepthRange(Object::RenderList const & render_list)
	{
		float float_max = std::numeric_limits<float>::max();
		RenderRange frustum_depth_range (float_max, - float_max);
		
		// For all leaf nodes in the render list,
		for (Object::RenderList::const_iterator i = render_list.begin(), end = render_list.end(); i != end; ++ i)
		{
			auto & object = * i;
			
			// and has a render range,
			RenderRange depth_range;
			if (! object.GetRenderRange(depth_range))
			{
				continue;
			}
			
			ASSERT(! IsInf(depth_range[0]));
			ASSERT(! IsInf(depth_range[1]));
			
			// and it isn't behind the camera,
			if (depth_range[1] <= camera_near)
			{
				continue;
			}
			
			// then include it in adjusted frustum render range.
			if (depth_range[0] < frustum_depth_range[0])
			{
				frustum_depth_range[0] = depth_range[0];
			}
			
			if (depth_range[1] > frustum_depth_range[1])
			{
				frustum_depth_range[1] = depth_range[1];
			}
		}

		// Bind the near plane to the default value,
		frustum_depth_range[0] = std::max(frustum_depth_range[0], (Scalar)camera_near);
		
		if (frustum_depth_range[0] >= frustum_depth_range[1])
		{
			// with nothing to render, there is no valid depth range,
			// so throw together some dummy values.
			return RenderRange(camera_near, camera_near * 1024);
		}
		
		return frustum_depth_range;
	}
	
	// Given the scene, adjusts its frustum with suitable near/far z values 
	// and applies it to the projection matrix.
	// TODO: move this to a utils file; RenderRange -> DepthRange
	Matrix44 CalcProjectionMatrix(Scene const & scene, RenderRange depth_range)
	{
		Pov const & pov = scene.GetPov();
		Frustum frustum = pov.GetFrustum();
		frustum.depth_range = depth_range;
		
		return frustum.CalcProjectionMatrix();
	}

	// Given the scene, adjusts its frustum with suitable near/far z values 
	// and applies it to the projection matrix.
	// TODO: belongs with Scene; scene should be const
	Matrix44 CalcForegroundProjectionMatrix(Scene const & scene)
	{
		auto & render_list = scene.GetRenderList();

		RenderRange depth_range = CalculateDepthRange(render_list);
		STAT_SET (z_range, depth_range);

		return CalcProjectionMatrix(scene, depth_range);
	}

	// Creates a frustum with sensible defaults for rendering a background.
	Matrix44 CalcBackgroundProjectionMatrix(Scene const & scene)
	{
		// Set projection matrix within relatively tight bounds.
		RenderRange depth_range = { .1f, 10.f };
		return CalcProjectionMatrix(scene, depth_range);
	}

}	// namespace

////////////////////////////////////////////////////////////////////////////////
// gfx::Engine member definitions

#if defined(__ANDROID__)
std::atomic<bool> Engine::_paused(false);
#else
const bool Engine::_paused(false);
#endif

Engine::StateParam const Engine::init_state[] =
{
	INIT_CAP(GL_CULL_FACE, true),
	INIT_CAP(GL_DEPTH_TEST, true),
	INIT_CAP(GL_BLEND, false),
};

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Engine, self)
	CRAG_VERIFY(core::StaticCast<super const>(self));
	CRAG_VERIFY(self.scene);
CRAG_VERIFY_INVARIANTS_DEFINE_END

Engine::Engine()
: scene(nullptr)
, _origin(geom::abs::Vector3::Zero())
, _target_frame_duration(.1)
, last_frame_end_position(app::GetTime())
, quit_flag(false)
, _ready(true)
, _dirty(true)
, culling(init_culling)
, capture_frame(0)
, _current_program(nullptr)
, _current_vbo(nullptr)
{
#if ! defined(NDEBUG)
	std::fill(std::begin(_frame_time_history), std::end(_frame_time_history), last_frame_end_position);
#endif
	
	if (! Init())
	{
		quit_flag = true;
	}
	
	RegisterResources();
}

Engine::~Engine()
{
	UnregisterResources();

	Deinit();

	// must be turned on by key input or by cfg edit
	capture_enable = false;
}

Scene & Engine::GetScene()
{
	ASSERT(Daemon::IsCurrentThread());
	
	return ref(scene);
}

Scene const & Engine::GetScene() const
{
	ASSERT(Daemon::IsCurrentThread());
	
	return ref(scene);
}

Program const * Engine::GetCurrentProgram() const
{
	return _current_program;
}

void Engine::SetCurrentProgram(Program const * program)
{
	if (program == _current_program)
	{
		return;
	}
	
	if (_current_program != nullptr)
	{
		_current_program->Unbind();
	}
	
	_current_program = program;
	
	if (_current_program != nullptr)
	{
		ASSERT(_current_program->IsInitialized());
		
		_current_program->Bind();
	}
	
	GL_VERIFY;
}

VboResource const * Engine::GetVboResource() const
{
	return _current_vbo;
}

void Engine::SetVboResource(VboResource const * vbo)
{
	if (vbo == _current_vbo)
	{
		return;
	}
	
	if (_current_vbo != nullptr)
	{
		_current_vbo->Deactivate();
	}
	
	_current_vbo = vbo;

	if (_current_vbo != nullptr)
	{
		_current_vbo->Activate();
	}
}

void Engine::OnQuit()
{
	quit_flag = true;
}

void Engine::OnAddObject(ObjectSharedPtr const & object_ptr)
{
	ASSERT(scene != nullptr);

	auto & object = * object_ptr;
	scene->AddObject(object);
	OnSetParent(object, Uid());
}

void Engine::OnRemoveObject(ObjectSharedPtr const & object_ptr)
{
	auto & object = * object_ptr;
	auto & children = object.GetChildren();
	while (! children.empty())
	{
		auto & back = children.back();
		DestroyObject(back.GetUid());
	}

	ASSERT(scene != nullptr);
	scene->RemoveObject(object);
}

void Engine::OnSetParent(Uid child_uid, Uid parent_uid)
{
	auto const & child = GetObject(child_uid);
	if (! child)
	{
		return;
	}
	
	OnSetParent(* child, parent_uid);
}

void Engine::OnSetParent(Object & child, Uid parent_uid)
{
	auto & parent = [&] () -> Object &
	{
		if (parent_uid)
		{
			auto & object = GetObject(parent_uid);
		
			if (object)
			{
				return * object;
			}
		}
		
		return scene->GetRoot();
	} ();
	
	OnSetParent(child, parent);
}

void Engine::OnSetParent(Object & child, Object & parent)
{
	OrphanChild(child);
	AdoptChild(child, parent);
}

void Engine::OnSetTime(Time time)
{
	scene->SetTime(time);
}

void Engine::OnSetReady(bool ready)
{
	ASSERT(ready != _ready);
	_ready = ready;
	_dirty = true;

	auto & resource_manager = crag::core::ResourceManager::Get();
	resource_manager.GetHandle<PolyProgram>("PolyProgram")->SetNeedsMatrixUpdate(true);
	resource_manager.GetHandle<DiskProgram>("SphereProgram")->SetNeedsMatrixUpdate(true);
	resource_manager.GetHandle<TexturedProgram>("SkyboxProgram")->SetNeedsMatrixUpdate(true);
}

void Engine::OnResize(geom::Vector2i size)
{
	glViewport(0, 0, size.x, size.y);
	scene->SetResolution(size);
}


void Engine::OnToggleCulling()
{
	culling = ! culling;
	_dirty = true;
}

void Engine::OnToggleCapture()
{
	capture_enable = ! capture_enable;
}

void Engine::operator() (SetCameraEvent const & event)
{
	if (scene != nullptr)
	{
		Transformation relative_transformation = geom::AbsToRel(event.transformation, _origin);
		scene->SetCameraTransformation(relative_transformation);
	}
}

void Engine::operator() (SetOriginEvent const & event)
{
	_origin = event.origin;
}

geom::abs::Vector3 const & Engine::GetOrigin() const
{
	return _origin;
}

#if defined(__ANDROID__)
void Engine::SetPaused(bool paused)
{
	_paused = paused;
}
#endif

void Engine::Run(Daemon::MessageQueue & message_queue)
{
	auto & children = scene->GetRoot().GetChildren();
	while (! quit_flag || ! children.empty())
	{
		CRAG_VERIFY(* scene);
		message_queue.DispatchMessages(* this);
		CRAG_VERIFY(* scene);
		
		if (! _paused && _ready && (_dirty || quit_flag))
		{
			PreRender();
			UpdateTransformations();
			UpdateShadowVolumes();
			Render();
			Capture();
			
			_dirty = false;
		}
		else
		{
			smp::Yield();
		}
	}

	SetCameraListener::SetIsListening(false);
	SetOriginListener::SetIsListening(false);
}

bool Engine::ProcessMessage(Daemon::MessageQueue & message_queue)
{
	return message_queue.DispatchMessage(* this);
}

bool Engine::Init()
{
	ASSERT(scene == nullptr);

	if (! app::InitContext())
	{
		return false;
	}
	
	if (! InitGlew())
	{
		return false;
	}
	
	InitVSync();
	
	scene = new Scene(* this);
	geom::Vector2i resolution = app::GetResolution();
	scene->SetResolution(resolution);
	
	InitRenderState();
	
	Debug::Init();
	return true;
}

void Engine::Deinit()
{
	if (scene == nullptr)
	{
		ASSERT(false);
		return;
	}
	
	Debug::Deinit();
	
	init_culling = culling;
	
	delete scene;
	scene = nullptr;

	app::DeinitContext();
}

// Decide whether to use vsync and initialize GL state accordingly.
void Engine::InitVSync()
{
	auto refresh_rate = app::GetRefreshRate();
	if (refresh_rate == 0)
	{
		DEBUG_MESSAGE("Unknown refresh rate; using default_refresh_rate of %dHz", default_refresh_rate);
		refresh_rate = default_refresh_rate;
	}

	_target_frame_duration = static_cast<float>(swap_interval) / refresh_rate;

	if (SDL_GL_SetSwapInterval(0) != 0)
	{
		DEBUG_BREAK_SDL();
	}
}

void Engine::InitRenderState()
{
	for (auto const & param : init_state)
	{
		GL_CALL((param.enabled ? glEnable : glDisable)(param.cap));
	}

	GL_CALL(glFrontFace(GL_CCW));
	GL_CALL(glCullFace(GL_BACK));
	glDepthFunc(depth_func);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	setDepthRange(0.f, max_foreground_depth);

	VerifyRenderState();
}

void Engine::VerifyRenderState() const
{
#if ! defined(NDEBUG)
	for (auto const & param : init_state)
	{
		if (param.enabled != IsEnabled(param.cap))
		{
			DEBUG_BREAK("Bad render state: %s.", param.name);
		}
	}
	
	// TODO: Write equivalent functions for all state in GL. :S
	ASSERT(GetInt<GL_DEPTH_FUNC>() == depth_func);
#endif	// NDEBUG
}

void Engine::PreRender()
{
	// purge objects
	auto & render_list = scene->GetRenderList();
	for (auto i = render_list.begin(), end = render_list.end(); i != end; )
	{
		auto & object = * i;
		++ i;
		
		auto result = object.PreRender();
		
		switch (result)
		{
			default:
				ASSERT(false);
			case Object::ok:
				break;
			case Object::remove:
				DestroyObject(object.GetUid());
				break;
		}
	}
}

void Engine::UpdateTransformations(Object & object, Transformation const & parent_model_view_transformation)
{
	// calculate model view transformation for this object
	auto & object_transformation = object.GetLocalTransformation();
	auto model_view_transformation = parent_model_view_transformation * object_transformation;
	CRAG_VERIFY(model_view_transformation);

	// if it's something that'll get drawn
	auto & children = object.GetChildren();
	if (object.GetParent())
	{
		// set model view transformation (with whatever necessary rejiggering)
		object.UpdateModelViewTransformation(model_view_transformation);
	}
	else
	{
		// if it's an empty branch,
		if (children.empty() && & object != & scene->GetRoot())
		{
			// destroy it
			DestroyObject(object.GetUid());
			return;
		}
	}
	
	// propagate to children
	for (auto i = children.begin(), end = children.end(); i != end;)
	{
		auto & child = * i;
		++ i;

		UpdateTransformations(child, model_view_transformation);
	}
}

void Engine::UpdateTransformations()
{
	Object & root_node = scene->GetRoot();
	UpdateTransformations(root_node, Transformation::Matrix44::Identity());
	
	scene->SortRenderList();
}

void Engine::UpdateShadowVolumes()
{
	auto & lights = scene->GetLightList();
	for (auto & light : lights)
	{
		light.SetIsExtinguished(false);
	}
	
	ShadowMap & shadows = scene->GetShadows();
	for (auto & pair : shadows)
	{
		auto & key = pair.first;
		auto & object = * key.first;
		auto & light = * key.second;
		
		ASSERT(light.GetException() != & object);

		auto & shadow = pair.second;

		if (! object.GenerateShadowVolume(light, shadow))
		{
			light.SetIsExtinguished(true);
		}
	}
}

void Engine::Render()
{
	RenderFrame();

	if (! _paused)
	{
		app::SwapBuffers();
	}

	ProcessRenderTiming();
}

void Engine::RenderFrame()
{
	VerifyRenderState();
	
	// clear the screen.
	ASSERT(GetBool<GL_DEPTH_WRITEMASK>());
	GL_CALL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	
	// TODO: Come up with system for these, more readable, self contained, include init_state
	if (! culling)
	{
		Disable(GL_CULL_FACE);
	}
	
	RenderScene();

	if (! culling)
	{
		Enable(GL_CULL_FACE);
	}

	VerifyRenderState();
}

void Engine::RenderScene()
{
	ASSERT(GetInt<GL_DEPTH_FUNC>() == depth_func);
	
	// calculate matrices
	auto foreground_projection_matrix = CalcForegroundProjectionMatrix(* scene);
	auto background_projection_matrix = CalcBackgroundProjectionMatrix(* scene);
	
	if (shadows_enabled)
	{
		// render foreground, opaque elements with non-shadow lighting
		auto light_filter = [] (Light const & light) { return light.GetAttributes().makes_shadow == false; };
		RenderLayer(foreground_projection_matrix, Layer::opaque, light_filter, true);
	
		// render foreground, opaque elements with shadow lighting
		RenderShadowLights(foreground_projection_matrix);
	}
	else
	{
		// render foreground, opaque elements with all lighting
		auto light_filter = [] (Light const &) { return true; };
		RenderLayer(foreground_projection_matrix, Layer::opaque, light_filter, true);
	}
	
	// render background elements (skybox)
	setDepthRange(0.f, 1.f);
	glDepthFunc(GL_LEQUAL);
	auto light_filter = [] (Light const & light) { return light.GetAttributes().type == LightType::search; };
	RenderLayer(background_projection_matrix, Layer::background, light_filter, true);
	glDepthFunc(depth_func);
	setDepthRange(0.f, max_foreground_depth);
	
	// render foreground, transparent elements
	RenderTransparentPass(foreground_projection_matrix);

#if defined (GATHER_STATS)
	DebugText();
#endif
}

void Engine::RenderTransparentPass(Matrix44 const & projection_matrix)
{	
	// render partially transparent objects
	Enable(GL_BLEND);
	glDepthMask(GL_FALSE);

	auto light_filter = [] (Light const &) { return true; };
	RenderLayer(projection_matrix, Layer::transparent, light_filter, true);

	// vbo needs to be reset before next frame
	// to ensure that FormationVbo::PreRender functions correctly
	SetVboResource(nullptr);

#if defined(CRAG_GFX_DEBUG)
	DebugDraw(projection_matrix);
#endif

	glDepthMask(GL_TRUE);
	Disable(GL_BLEND);
}

void Engine::RenderLayer(Matrix44 const & projection_matrix, Layer layer, LightFilter const & light_filter, bool add_ambient)
{
	// mark all (relevant) shaders as having out-of-date light uniforms
	auto & resource_manager = crag::core::ResourceManager::Get();
	resource_manager.GetHandle<PolyProgram>("PolyProgram")->SetNeedsLightsUpdate(true);
	resource_manager.GetHandle<DiskProgram>("SphereProgram")->SetNeedsLightsUpdate(true);
	resource_manager.GetHandle<TexturedProgram>("SkyboxProgram")->SetNeedsLightsUpdate(true);

	auto ambient = add_ambient ? global_ambient : Color4f::Black();
	auto & lights = scene->GetLightList();
	
	auto & render_list = scene->GetRenderList();
	for (auto & object : render_list)
	{
		if (object.GetLayer() != layer)
		{
			continue;
		}
		
		// if object 'cares' what shader is enabled
		auto required_program = object.GetProgram();
		if (required_program)
		{
			// make it current (binds if necessary)
			SetCurrentProgram(required_program);
		
			// Set the model view matrix.
			Transformation const & model_view_transformation = object.GetModelViewTransformation();
			auto & model_view_matrix = model_view_transformation.GetMatrix();
			required_program->SetModelViewMatrix(model_view_matrix);

			// update projectin matrix
			if (required_program->NeedsMatrixUpdate())
			{
				required_program->SetNeedsMatrixUpdate(false);
				
				// happens once per shader per frame
				required_program->SetProjectionMatrix(projection_matrix);
			}

			// update projectin matrix
			if (required_program->NeedsLightsUpdate())
			{
				required_program->SetNeedsLightsUpdate(false);
				
				// happens once per shader per call to RenderLayer
				required_program->SetLights(ambient, lights, light_filter);
			}
		}
		
		auto required_vbo = object.GetVboResource();
		if (required_vbo)
		{
			SetVboResource(required_vbo);
		}
		
		object.Render(* this);
	}
}

void Engine::RenderShadowLights(Matrix44 const & projection_matrix)
{
	Enable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	auto & lights = scene->GetLightList();
	for (auto & light : lights)
	{
		if (light.GetAttributes().makes_shadow && light.GetIsLuminant())
		{
			RenderShadowLight(projection_matrix, light);
		}
	}
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);
	Disable(GL_STENCIL_TEST);
}

void Engine::RenderShadowLight(Matrix44 const & projection_matrix, Light & light)
{
	GL_VERIFY;

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	// shadow volume

	GL_CALL(glClear(GL_STENCIL_BUFFER_BIT));

	auto & resource_manager = crag::core::ResourceManager::Get();
	auto & shadow_program = * resource_manager.GetHandle<ShadowProgram>("ShadowProgram");
	SetCurrentProgram(& shadow_program);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_ALWAYS, 1, 0xFFFFFFFFL);
	glDepthFunc(GL_LEQUAL);

	////////////////////////////////////////////////////////////////////////////////
	// back
	
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	RenderShadowVolumes(projection_matrix, light);

	////////////////////////////////////////////////////////////////////////////////
	// front

	glFrontFace(GL_CW);
	glStencilOp(GL_KEEP, GL_KEEP, GL_DECR);
	RenderShadowVolumes(projection_matrix, light);
	glFrontFace(GL_CCW);

	////////////////////////////////////////////////////////////////////////////////
	// reset some stuff

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	// light

	// program
	auto & screen_program = * resource_manager.GetHandle<ScreenProgram>("ScreenProgram");
	SetCurrentProgram(& screen_program);

	// state
	glStencilFunc(GL_EQUAL, 0, 0xFFFFFFFFL);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	// render
	glDepthFunc(GL_LEQUAL);
	Enable(GL_BLEND);	// TODO: Ensure additive; move this call further down stack

	auto light_filter = [& light] (Light const & l) { return & l == & light; };
	RenderLayer(projection_matrix, Layer::opaque, light_filter, false);

	Disable(GL_BLEND);
	glDepthFunc(depth_func);

#if defined(CRAG_DEBUG_SHADOW_VOLUMES)
	// draw shadow volumes to color buffer for diagnostic purposes
	bool flip = (int(app::GetTime() * 256) & 255) == 0;
	if (flip)
	{
		glFrontFace(GL_CW);
	}
	glDepthFunc(GL_ALWAYS);
	SetCurrentProgram(& shadow_program);
	glDepthMask(GL_TRUE);
	Enable(GL_BLEND);
	Disable(GL_STENCIL_TEST);
//	Disable(GL_CULL_FACE);
	RenderShadowVolumes(projection_matrix, light);
//	Enable(GL_CULL_FACE);
	Enable(GL_STENCIL_TEST);
	Disable(GL_BLEND);
	glDepthMask(GL_FALSE);
	glDepthFunc(depth_func);
	if (flip)
	{
		glFrontFace(GL_CCW);
	}
#endif

	GL_VERIFY;
}

void Engine::RenderShadowVolumes(Matrix44 const & projection_matrix, Light & light)
{
	auto & resource_manager = crag::core::ResourceManager::Get();
	auto & shadow_program = * resource_manager.GetHandle<ShadowProgram>("ShadowProgram");
	ASSERT(GetCurrentProgram() == & shadow_program);

	shadow_program.SetProjectionMatrix(projection_matrix);
	
	auto & shadows = scene->GetShadows();
	auto & render_list = scene->GetRenderList();

	ShadowMapKey key;
	key.second = & light;
	auto exception = light.GetException();
	for (auto & object : render_list)
	{
		if (! object.CastsShadow() || & object == exception)
		{
			continue;
		}
		
		if (object.GetLayer() != Layer::opaque)
		{
			continue;
		}
		
		// get shadow that matches the object-light combination
		key.first = & object;
		auto found = shadows.find(key);
		ASSERT(found != std::end(shadows));
		
		auto & vbo_resource = found->second;
		if (! vbo_resource.IsInitialized())
		{
			continue;
		}
		
		SetVboResource(& vbo_resource);

		// Set the model view matrix.
		auto & model_view_transformation = object.GetShadowModelViewTransformation();
		auto & model_view_matrix = model_view_transformation.GetMatrix();
		shadow_program.SetModelViewMatrix(model_view_matrix);

		// Draw
		vbo_resource.Draw();
	}
}

#if defined(CRAG_GFX_DEBUG)
void Engine::DebugDraw(Matrix44 const & projection_matrix)
{
	if (capture_enable)
	{
		return;
	}

	SetCurrentProgram(nullptr);
	
	// calculate model view transformation
	auto & pov = scene->GetPov();
	auto & model_view_projection = pov.GetTransformation();

	// mark the local origin
	Debug::AddBasis(Vector3::Zero(), 1000000.);
	
	// draw 3D debug elements
	auto model_view_matrix = model_view_projection.GetMatrix();

	glDepthMask(GL_FALSE);
	if (culling)
	{
		Disable(GL_CULL_FACE);
		Debug::Draw(model_view_matrix, projection_matrix);
		Enable(GL_CULL_FACE);
	}
	else
	{
		Debug::Draw(model_view_matrix, projection_matrix);
	}
	glDepthMask(GL_TRUE);
	
	Debug::Clear();
	
	// print camera position
	STAT_SET (pos, model_view_projection.GetTranslation());
}
#endif	// defined(CRAG_GFX_DEBUG)

#if defined (GATHER_STATS)
void Engine::DebugText()
{
	// The string into which is written Debug text.
	std::stringstream out_stream;
	
	for (core::Statistics::iterator i = core::Statistics::begin(); i != core::Statistics::end(); ++ i)
	{
		core::StatInterface const & stat = * i;
		if (Debug::GetVerbosity() > stat.GetVerbosity())
		{
			out_stream << stat.GetName() << ": " << stat << '\n';
		}
	}
	
	auto & resource_manager = crag::core::ResourceManager::Get();
	auto const & sprite_program = * resource_manager.GetHandle<SpriteProgram>("SpriteProgram");
	SetCurrentProgram(& sprite_program);
	sprite_program.SetUniforms(app::GetResolution());
	Debug::DrawText(out_stream.str().c_str(), geom::Vector2i(5, 5));
}
#endif	// defined (GATHER_STATS)

void Engine::ProcessRenderTiming()
{
	Time frame_start_position, frame_end_position;
	GetRenderTiming(frame_start_position, frame_end_position);
	
	Time frame_duration = frame_end_position - frame_start_position;
	STAT_SET(frame_duration, frame_duration);
	
#if defined(GATHER_STATS)
	UpdateFpsCounter(frame_start_position);
#endif
	
	SampleFrameDuration(frame_duration);
}

// TODO: Consider SDL_GetPerformanceCounter / SDL_GetPerformanceFrequency
void Engine::GetRenderTiming(Time & frame_start_position, Time & frame_end_position)
{
	frame_start_position = last_frame_end_position;
	frame_end_position = app::GetTime();

	ASSERT(frame_end_position >= frame_start_position);

	last_frame_end_position = frame_end_position;
}

#if defined(GATHER_STATS) && ! defined(NDEBUG)
void Engine::UpdateFpsCounter(Time frame_start_position)
{
	// update the history
	auto begin = std::begin(_frame_time_history);
	auto end = std::end(_frame_time_history);
	std::copy(begin + 1, end, begin);

	// get the range
	Time first_entry = * begin;
	Time & last_entry = * (end - 1);
	last_entry = frame_start_position;
	
	// average it
	float history_duration = _frame_time_history_size - 1;
	float average_frame_duration = float(last_entry - first_entry) / history_duration;
	STAT_SET (fps, 1.f / average_frame_duration);
}
#endif

void Engine::SampleFrameDuration(Time frame_duration) const
{
	float frame_duration_ratio = float(frame_duration / _target_frame_duration);
	ASSERT(frame_duration_ratio >= 0);
	ASSERT(! IsInf(frame_duration_ratio));
	
	gfx::FrameDurationSampledMessage message = { frame_duration_ratio };
	Daemon::Broadcast(message);
}

void Engine::Capture()
{
	if (! capture_enable)
	{
		return;
	}
	
	// Standard string/stream classes: 1/10.
	std::ostringstream filename_stream;
	filename_stream << "../../../" << capture_frame << ".bmp";
	std::string filename_string = filename_stream.str();
	
	bool success = capture_image[0].CaptureScreen()
		&& Image::CopyVFlip(capture_image[1], capture_image[0])
		&& capture_image[1].Save(filename_string.c_str());
	
	if (! success)
	{
		capture_enable = false;
		return;
	}

	++ capture_frame;
}

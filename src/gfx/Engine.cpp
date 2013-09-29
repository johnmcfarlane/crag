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
#include "MeshResource.h"
#include "Messages.h"
#include "Program.h"
#include "ResourceManager.h"
#include "Scene.h"
#include "SetCameraEvent.h"
#include "SetOriginEvent.h"

#include "form/Engine.h"

#include "sim/Engine.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/Statistics.h"

using core::Time;
using namespace gfx;

CONFIG_DEFINE(depth_func, int, GL_LEQUAL);
CONFIG_DECLARE(profile_mode, bool);
CONFIG_DECLARE(camera_near, float);

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// File-local Variables
	
	//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
	CONFIG_DEFINE (background_ambient_color, Color4f, Color4f(0.1f));
	CONFIG_DEFINE (target_work_proportion, double, .95f);
	CONFIG_DEFINE (default_refresh_rate, int, 50);

	CONFIG_DEFINE (init_culling, bool, true);
	CONFIG_DEFINE (init_flat_shaded, bool, false);
#if defined(CRAG_USE_GLES)
	CONFIG_DEFINE (init_fragment_lighting, bool, false);
#else
	CONFIG_DEFINE (init_fragment_lighting, bool, true);
#endif

	CONFIG_DEFINE (enable_shadow_mapping, bool, true);
	
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

	STAT (frame_1_busy_duration, double, .15f);
	STAT (frame_2_vsync_duration, double, .18f);
	STAT (frame_3_total_duration, double, .18f);
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
	
	bool GlSupportsFences()
	{
#if defined(__APPLE__)
		return true;
#elif defined(__ANDROID__)
		return false;
#else
		return GLEW_NV_fence != GL_FALSE;
#endif
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
	RenderRange CalculateDepthRange(LeafNode::RenderList const & render_list)
	{
		float float_max = std::numeric_limits<float>::max();
		RenderRange frustum_depth_range (float_max, - float_max);
		
		// For all leaf nodes in the render list,
		for (LeafNode::RenderList::const_iterator i = render_list.begin(), end = render_list.end(); i != end; ++ i)
		{
			LeafNode const & leaf_node = * i;
			
			// if in the foreground layer,
			if (leaf_node.GetLayer() != Layer::foreground)
			{
				continue;
			}
			
			// and has a render range,
			RenderRange depth_range;
			if (! leaf_node.GetRenderRange(depth_range))
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
		LeafNode::RenderList const & render_list = scene.GetRenderList();

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
	
	// Given a scene and the uid of a branc_node in that scene
	// (or Uid() for the scene's root node),
	// returns a reference to that branch node.
	Object * GetParent(Engine & engine, Uid parent_uid)
	{
		if (parent_uid)
		{
			auto * parent = engine.GetObject(parent_uid);

			if (parent != nullptr)
			{
				return parent;
			}

			DEBUG_BREAK("missing object");
		}

		auto & scene = engine.GetScene();
		return & scene.GetRoot();
	}

}	// namespace

////////////////////////////////////////////////////////////////////////////////
// gfx::Engine member definitions

#if defined(VERIFY)
void Engine::Verify() const
{
	super::Verify();

	VerifyObjectPtr(scene);
}
#endif

Engine::Engine()
: scene(nullptr)
, _resource_manager(nullptr)
, _origin(geom::abs::Vector3::Zero())
, _frame_duration(0)
, last_frame_end_position(app::GetTime())
, quit_flag(false)
, _ready(true)
, _dirty(true)
, vsync(false)
, culling(init_culling)
, _flat_shaded(init_flat_shaded)
, _fragment_lighting(init_fragment_lighting)
, capture_frame(0)
, _current_program(nullptr)
, _current_mesh(nullptr)
{
#if ! defined(NDEBUG)
	std::fill(_frame_time_history, _frame_time_history + _frame_time_history_size, 0);
#endif
	
	if (! Init())
	{
		quit_flag = true;
	}
}

Engine::~Engine()
{
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

ResourceManager & Engine::GetResourceManager()
{
	return ref(_resource_manager);
}

ResourceManager const & Engine::GetResourceManager() const
{
	return ref(_resource_manager);
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

MeshResource const * Engine::GetCurrentMesh() const
{
	return _current_mesh;
}

void Engine::SetCurrentMesh(MeshResource const * mesh)
{
	if (mesh == _current_mesh)
	{
		return;
	}
	
	if (_current_mesh != nullptr)
	{
		_current_mesh->Deactivate();
	}
	
	_current_mesh = mesh;

	if (_current_mesh != nullptr)
	{
		_current_mesh->Activate();
	}
}

Color4f Engine::CalculateLighting(Vector3 const & position) const
{
	Color4f lighting_color = Color4f::Black();
	
	Light::List const & lights = scene->GetLightList();
	for (Light::List::const_iterator i = lights.begin(), end = lights.end(); i != end; ++ i)
	{
		Light const & light = * i;

		Vector3 light_position = light.GetModelViewTransformation().GetTranslation();
		
		Vector3 frag_to_light = light_position - position;
		float distance_squared = static_cast<float>(LengthSq(frag_to_light));
		
		float attenuation = Clamped(1.f / distance_squared, 0.f, 1.f);
		
		Color4f const & color = light.GetColor();
		ASSERT(color.a == 1.f);
		Color4f diffuse = color * attenuation;
		
		lighting_color += diffuse;
	}
	
	lighting_color.a = 1.f;
	return lighting_color;
}

void Engine::OnQuit()
{
	quit_flag = true;
}

void Engine::OnAddObject(Object & object)
{
	ASSERT(scene != nullptr);

	// success!
	scene->AddObject(object);
	OnSetParent(object, Uid());
}

void Engine::OnRemoveObject(Object & object)
{
	while (! object.IsEmpty())
	{
		auto & back = object.Back();
		DestroyObject(back.GetUid());
	}

	ASSERT(scene != nullptr);
	scene->RemoveObject(object);
}

void Engine::OnSetParent(Uid child_uid, Uid parent_uid)
{
	auto * child = GetObject(child_uid);
	if (child == nullptr)
	{
		return;
	}
	
	OnSetParent(ref(child), parent_uid);
}

void Engine::OnSetParent(Object & child, Uid parent_uid)
{
	auto * parent = GetParent(* this, parent_uid);
	if (parent == nullptr)
	{
		DEBUG_BREAK("Given parent, " SIZE_T_FORMAT_SPEC ", not found", std::hash<Uid>()(parent_uid));
		return;
	}
	
	OnSetParent(child, * parent);
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

void Engine::SetFlatShaded(bool flat_shaded)
{
	_dirty = _flat_shaded != flat_shaded;
	_flat_shaded = flat_shaded;
}

bool Engine::GetFlatShaded() const
{
	return _flat_shaded;
}

void Engine::SetFragmentLighting(bool fragment_lighting)
{
	_dirty = _fragment_lighting != fragment_lighting;
	_fragment_lighting = fragment_lighting;
}

bool Engine::GetFragmentLighting() const
{
	return _fragment_lighting;
}

void Engine::OnToggleCapture()
{
	capture_enable = ! capture_enable;
}

void Engine::operator() (SetCameraEvent const & event)
{
	if (scene != nullptr)
	{
		scene->SetCameraTransformation(event.transformation);
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

#if defined(NDEBUG)
#define INIT(CAP,ENABLED) { CAP,ENABLED }
#else
#define INIT(CAP,ENABLED) { CAP,ENABLED,#CAP }
#endif

Engine::StateParam const Engine::init_state[] =
{
	INIT(GL_CULL_FACE, true),
	INIT(GL_DEPTH_TEST, true),
	INIT(GL_BLEND, false),
	INIT(GL_INVALID_ENUM, false),
};

void Engine::Run(Daemon::MessageQueue & message_queue)
{
	while (! quit_flag || ! scene->GetRoot().IsEmpty())
	{
		message_queue.DispatchMessages(* this);
		VerifyObjectRef(* scene);
		
		if (_ready && (_dirty || quit_flag))
		{
			PreRender();
			UpdateTransformations();
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
	
	if (! InitFrameBuffer())
	{
		return false;
	}
	
	_resource_manager = new ResourceManager;
	
	InitVSync();
	
	scene = new Scene(* this);
	geom::Vector2i resolution = app::GetResolution();
	scene->SetResolution(resolution);
	
	InitRenderState();
	
	Debug::Init();
	return true;
}

bool Engine::InitFrameBuffer()
{
//	frame_buffer.Init();
//	frame_buffer.Bind();
//	
//	depth_buffer.Init();
//	depth_buffer.Bind();
//	geom::Vector2i resolution = app::GetResolution();
//	depth_buffer.ResizeForDepth(resolution.x, resolution.y);
//	
//	depth_texture.Init();
//	depth_texture.Bind();
//	depth_texture.SetImage(resolution.x, resolution.y, nullptr); 
//	
//	Attach(frame_buffer, depth_texture);
	
	return true;
}

void Engine::Deinit()
{
	if (scene == nullptr)
	{
		ASSERT(false);
		return;
	}
	
	delete _resource_manager;
	_resource_manager = nullptr;
	
	Debug::Deinit();
	
	init_culling = culling;
	
	if (_fence2.IsInitialized())
	{
		_fence2.Deinit();
	}

	if (_fence1.IsInitialized())
	{
		_fence1.Deinit();
	}
	
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

	_frame_duration = 1.0f / refresh_rate;

	if (profile_mode)
	{
		vsync = false;
	}
	else
	{
		if (GlSupportsFences())
		{
			_fence1.Init();
			_fence2.Init();

			vsync = _fence1.IsInitialized() && _fence2.IsInitialized();
		}
		else
		{
			vsync = false;
		}
	}

	int desired_swap_interval = vsync ? 1 : 0;

	if (SDL_GL_SetSwapInterval(desired_swap_interval) != 0)
	{
		DEBUG_BREAK_SDL();

		// It's hard to tell whether vsync is enabled or not at this point.
		// Assume that it's not.
		vsync = false;
	}
	else
	{
		// Note: Oddly, when vsync is forced on in NVidia Control Panel,
		// it still gets turned off by call to SDL_GL_SetSwapInterval.
		int actual_swap_interval = SDL_GL_GetSwapInterval();
		if (actual_swap_interval == desired_swap_interval)
		{
			// success
			return;
		}

		switch (actual_swap_interval)
		{
		case 0:
			break;

		case 1:
			ERROR_MESSAGE("Crag cannot run when vsync is forced on AND OpenGL feature, fences, are not supported.");
			exit(1);
			
		default:
			ASSERT(false);

		case -1:
			DEBUG_BREAK_SDL();
		}

		ASSERT(vsync);
	}

	// Something went wrong if we get to here.
	// We need to remove the fences and forgo vsync.

	vsync = false;
	if (_fence1.IsInitialized())
	{
		_fence1.Deinit();
	}
	if (_fence2.IsInitialized())
	{
		_fence2.Deinit();
	}
}

void Engine::InitRenderState()
{
	StateParam const * param = init_state;
	while (param->cap != GL_INVALID_ENUM)
	{
		GL_CALL((param->enabled ? glEnable : glDisable)(param->cap));
		++ param;
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
	StateParam const * param = init_state;
	while (param->cap != GL_INVALID_ENUM)
	{
#if ! defined(NDEBUG)
		if (param->enabled != IsEnabled(param->cap))
		{
			DEBUG_BREAK("Bad render state: %s.", param->name);
		}
#endif
		++ param;
	}
	
	// TODO: Write equivalent functions for all state in GL. :S
	ASSERT(GetInt<GL_DEPTH_FUNC>() == depth_func);
#endif	// NDEBUG
}

// TODO: Remove?
bool Engine::HasShadowSupport() const
{
	if (! enable_shadow_mapping) {
		return false;
	}
	
#if defined(__ANDROID__)
	return false;
#elif defined(__APPLE__)
	return true;
#else
	if (! GLEW_ARB_shadow) {
		return false;
	}
	if (! GLEW_ARB_depth_texture) {
		return false;
	}
	if (! GLEW_EXT_framebuffer_object) {
		return false;
	}
	
	return true;
#endif
}

void Engine::PreRender()
{
	// purge objects
	typedef LeafNode::RenderList List;
	
	List const & render_list = scene->GetRenderList();
	for (List::iterator i = render_list.begin(), end = render_list.end(); i != end; )
	{
		LeafNode & leaf_node = * i;
		++ i;
		
		LeafNode::PreRenderResult result = leaf_node.PreRender();
		
		switch (result)
		{
			default:
				ASSERT(false);
			case LeafNode::ok:
				break;
			case LeafNode::remove:
				DestroyObject(leaf_node.GetUid());
				break;
		}
	}
}

void Engine::UpdateTransformations(Object & object, Transformation const & parent_model_view_transformation)
{
	// calculate model view transformation for this object
	auto & object_transformation = object.GetLocalTransformation();
	auto model_view_transformation = parent_model_view_transformation * object_transformation;
	VerifyObject(model_view_transformation);

	// if it's something that'll get drawn
	auto * leaf = object.CastLeafNodePtr();
	if (leaf != nullptr)
	{
		// set model view transformation (with whatever necessary rejiggering)
		leaf->UpdateModelViewTransformation(model_view_transformation);
	}
	else
	{
		// if it's an empty branch,
		if (object.IsEmpty())
		{
			// destroy it
			DestroyObject(object.GetUid());
			return;
		}
	}
	
	// propagate to children
	for (auto i = object.Begin(), end = object.End(); i != end;)
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

void Engine::Render()
{
	RenderFrame();

	// Flip the front and back buffers and set fences.
	SetFence(_fence1);
	app::SwapBuffers();
	SetFence(_fence2);

	ProcessRenderTiming();
}

void Engine::RenderFrame()
{
	VerifyRenderState();
	
	// clear the screen.
	ASSERT(GetBool<GL_DEPTH_WRITEMASK>());
	GL_CALL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	
	// Set up lights.
	InvalidateUniforms();
	
	// All the things.
	if (culling)
	{
		RenderScene();
	}
	else
	{
		Disable(GL_CULL_FACE);
		RenderScene();
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
	
	// render forground, opaque elements
	RenderLayer(foreground_projection_matrix, Layer::foreground);
	
	// render background elements (skybox)
	setDepthRange(0.f, 1.f);
	RenderLayer(background_projection_matrix, Layer::background);
	setDepthRange(0.f, max_foreground_depth);
	
	// render forground, transparent elements
	RenderTransparentPass(foreground_projection_matrix);

#if defined (GATHER_STATS)
	DebugText();
#endif
}

void Engine::InvalidateUniforms()
{
	for (int program_index = 0; program_index != int(ProgramIndex::size); ++ program_index)
	{
		Program * program = _resource_manager->GetProgram(static_cast<ProgramIndex>(program_index));
		program->SetUniformsValid(false);
	}
}

void Engine::RenderTransparentPass(Matrix44 const & projection_matrix)
{	
	// render partially transparent objects
	Enable(GL_BLEND);
	glDepthMask(false);

	RenderLayer(projection_matrix, Layer::foreground, false);

	// mesh needs to be reset before next frame
	// to ensure that FormationMesh::PreRender functions correctly
	SetCurrentMesh(nullptr);

#if defined(CRAG_GFX_DEBUG)
	DebugDraw(projection_matrix);
#endif

	glDepthMask(true);
	Disable(GL_BLEND);
}

int Engine::RenderLayer(Matrix44 const & projection_matrix, Layer layer, bool opaque)
{
	int num_rendered_objects = 0;
	
	typedef LeafNode::RenderList List;
	List & render_list = scene->GetRenderList();
	auto & lights = scene->GetLightList();
	
	for (List::iterator i = render_list.begin(), end = render_list.end(); i != end; ++ i)
	{
		LeafNode & leaf_node = * i;
		
		if (leaf_node.GetLayer() != layer)
		{
			continue;
		}
		
		if (leaf_node.IsOpaque() != opaque)
		{
			continue;
		}
		
		Program * required_program = leaf_node.GetProgram();
		if (required_program != nullptr)
		{
			if (required_program->IsInitialized())
			{
				SetCurrentProgram(required_program);
				
				// once per frame,
				if (! required_program->GetUniformsValid())
				{
					required_program->SetUniformsValid(true);

					// set the frame-constant uniforms while it's active
					required_program->UpdateLights(lights);
					required_program->SetProjectionMatrix(projection_matrix);
				}
				
				// Set the model view matrix.
				Transformation const & model_view_transformation = leaf_node.GetModelViewTransformation();
				auto model_view_matrix = model_view_transformation.GetMatrix();
				required_program->SetModelViewMatrix(model_view_matrix);
			}
			else
			{
				SetCurrentProgram(nullptr);
			}
		}
		
		MeshResource const * required_mesh = leaf_node.GetMeshResource();
		if (required_mesh != nullptr)
		{
			SetCurrentMesh(required_mesh);
		}
		
		leaf_node.Render(* this);
		++ num_rendered_objects;
	}
	
	return num_rendered_objects;
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
	
	Program * sprite_program = _resource_manager->GetProgram(ProgramIndex::sprite);
	if (sprite_program != nullptr)
	{
		SetCurrentProgram(sprite_program);
		static_cast<SpriteProgram*>(sprite_program)->SetUniforms(app::GetResolution());
		Debug::DrawText(out_stream.str().c_str(), geom::Vector2i(5, 5));
	}
}
#endif	// defined (GATHER_STATS)

void Engine::ProcessRenderTiming()
{
	Time frame_start_position, pre_sync_position, post_sync_position;
	GetRenderTiming(frame_start_position, pre_sync_position, post_sync_position);
	
	Time frame_duration, busy_duration;
	ConvertRenderTiming(frame_start_position, pre_sync_position, post_sync_position, frame_duration, busy_duration);
	
#if defined(GATHER_STATS)
	UpdateFpsCounter(frame_start_position);
#endif
	
	SampleFrameDuration(busy_duration);
}

// TODO: Consider SDL_GetPerformanceCounter / SDL_GetPerformanceFrequency
void Engine::GetRenderTiming(Time & frame_start_position, Time & pre_sync_position, Time & post_sync_position)
{
	frame_start_position = last_frame_end_position;
	
	if (vsync)
	{
		// Get timing information from the fences.
		_fence1.Finish();
		pre_sync_position = app::GetTime();

		_fence2.Finish();
		post_sync_position = app::GetTime();
	}
	else
	{
		// There is no sync.
		pre_sync_position = post_sync_position = app::GetTime();
	}

	ASSERT(pre_sync_position >= frame_start_position);
	ASSERT(post_sync_position >= pre_sync_position);

	last_frame_end_position = post_sync_position;
}

void Engine::ConvertRenderTiming(Time frame_start_position, Time pre_sync_position, Time post_sync_position, Time & frame_duration, Time & busy_duration)
{
	Time vsync_duration = post_sync_position - pre_sync_position;
	Time frame_end_position = post_sync_position;
	
	frame_duration = frame_end_position - frame_start_position;
	busy_duration = frame_duration - vsync_duration;
	
	STAT_SET(frame_1_busy_duration, busy_duration);
	STAT_SET(frame_2_vsync_duration, vsync_duration);
	STAT_SET(frame_3_total_duration, frame_duration);
}

#if defined(GATHER_STATS) && ! defined(NDEBUG)
void Engine::UpdateFpsCounter(Time frame_start_position)
{
	// update the history
	memmove(_frame_time_history, _frame_time_history + 1, sizeof(* _frame_time_history) * (_frame_time_history_size - 1));
	Time first_entry = _frame_time_history[0];
	Time & last_entry = _frame_time_history[_frame_time_history_size - 1];
	last_entry = frame_start_position;
	
	// average it
	float history_duration = _frame_time_history_size - 1;
	float average_frame_duration = float(last_entry - first_entry) / history_duration;
	STAT_SET (fps, 1.f / average_frame_duration);
}
#endif

void Engine::SampleFrameDuration(Time busy_duration) const
{
	Time target_frame_duration = _frame_duration;
	if (vsync)
	{
		target_frame_duration *= target_work_proportion;
	}
	
	float frame_duration_ratio = float(busy_duration / target_frame_duration);
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

void Engine::SetFence(Fence & fence)
{
	if (fence.IsInitialized())
	{
		fence.Set();
	}
}

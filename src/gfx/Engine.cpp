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
#include "Program.h"
#include "ResourceManager.h"
#include "Scene.h"

#include "form/Engine.h"
#include "form/scene/RegulatorScript.h"

#include "sim/Engine.h"

#include "applet/Engine.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/Statistics.h"


using namespace gfx;


CONFIG_DEFINE (multisample, bool, false);
CONFIG_DECLARE (profile_mode, bool);
CONFIG_DECLARE(camera_near, float);


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// File-local Variables
	
	CONFIG_DEFINE (gfx_frame_duration, Time, 1.f / 60.f);

	//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
	CONFIG_DEFINE (background_ambient_color, Color4f, Color4f(0.1f));
	CONFIG_DEFINE (target_work_proportion, double, .95f);

	CONFIG_DEFINE (init_culling, bool, true);
	CONFIG_DEFINE (init_lighting, bool, true);
	CONFIG_DEFINE (init_wireframe, bool, false);

	CONFIG_DEFINE (enable_shadow_mapping, bool, true);
	
	CONFIG_DEFINE (capture_enable, bool, false);
	CONFIG_DEFINE (capture_skip, int, 0);
	
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
	
	
	////////////////////////////////////////////////////////////////////////////////
	// File-local functions
	
	bool InitGlew()
	{
#if defined(GLEW_STATIC)
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK)
		{
			std::cerr << "GLEW Error: " << glewGetErrorString(glew_err) << std::endl;
			return false;
		}
		
		std::cout << "GLEW Version: " << glewGetString(GLEW_VERSION) << std::endl;
		
#if ! defined(__APPLE__)
		if (! GLEW_VERSION_1_5)
		{
			std::cerr << "Error: Crag requires OpenGL 1.5 or greater." << std::endl;
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
#else
		return GLEW_NV_fence != GL_FALSE;
#endif
	}
	
	void SetModelViewMatrix(gfx::Transformation const & model_view_transformation)
	{
		ASSERT(GetInt<GL_MATRIX_MODE>() == GL_MODELVIEW);

		Matrix44 gl_model_view_matrix = model_view_transformation.GetOpenGlMatrix();
		LoadMatrix(gl_model_view_matrix);
	}
	
	// Creates a frustum with sensible defaults for rendering a background.
	void SetBackgroundFrustum(Pov const & pov)
	{
		// Set projection matrix within relatively tight bounds.
		Frustum background_frustum = pov.GetFrustum();
		background_frustum.depth_range[0] = .1f;
		background_frustum.depth_range[1] = 10.f;
		background_frustum.SetProjectionMatrix();
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
	void SetForegroundFrustum(Scene & scene)
	{
		Pov & pov = scene.GetPov();
		LeafNode::RenderList const & render_list = scene.GetRenderList();
		Frustum & frustum = pov.GetFrustum();

		frustum.depth_range = CalculateDepthRange(render_list);
		
		frustum.SetProjectionMatrix();
	}

	// Given a scene and the uid of a branc_node in that scene
	// (or Uid() for the scene's root node),
	// returns a reference to that branch node.
	BranchNode * GetBranchNode(Scene & scene, Uid branch_node_uid)
	{
		if (! branch_node_uid)
		{
			return & scene.GetRoot();
		}
		
		Object * parent_object = scene.GetObject(branch_node_uid);
		if (parent_object == nullptr)
		{
			// The given parent was not found.
			// The uid is bogus or the object has been removed.
			return nullptr;
		}
		
		if (parent_object->GetNodeType() != Object::branch)
		{
			// The given uid refers to an object which is not a BranchNode.
			// The uid is bogus.
			ASSERT(false);
			return nullptr;
		}
		
		return static_cast<BranchNode *>(parent_object);
	}

}	// namespace


Engine::Engine()
: context(nullptr)
, scene(nullptr)
, last_frame_end_position(app::GetTime())
, quit_flag(false)
, vsync(false)
, _ready(true)
, culling(init_culling)
, lighting(init_lighting)
, wireframe(init_wireframe)
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

Object * Engine::GetObject(Uid uid)
{
	return scene->GetObject(uid);
}

Object const * Engine::GetObject(Uid uid) const
{
	return scene->GetObject(uid);
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
		
		// In case this is the first time in this frame that the program has been set active,
		// set the lights while it's active now.
		Light::List const & lights = scene->GetLightList();
		_current_program->UpdateLights(lights);
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
	_ready = true;
}

void Engine::OnAddObject(Object & object)
{
	if (scene == nullptr)
	{
		delete & object;
		return;
	}

	// success!
	scene->AddObject(object);
	OnSetParent(object, Uid());
}

void Engine::OnRemoveObject(Uid uid)
{
	if (scene != nullptr)
	{
		scene->RemoveObject(uid);
	}
}

void Engine::OnSetParent(Uid child_uid, Uid parent_uid)
{
	Object * child = GetObject(child_uid);
	if (child == nullptr)
	{
		ASSERT(false);
		return;
	}
	
	OnSetParent(ref(child), parent_uid);
}

void Engine::OnSetParent(Object & child, Uid parent_uid)
{
	BranchNode * parent = GetBranchNode(ref(scene), parent_uid);
	if (parent == nullptr)
	{
		DEBUG_BREAK("Given parent, %tu, not found", parent_uid.GetValue());
		return;
	}
	
	OnSetParent(child, * parent);
}

void Engine::OnSetParent(Object & child, BranchNode & parent)
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
	_ready = ready;
}

void Engine::OnResize(geom::Vector2i size)
{
	glViewport(0, 0, size.x, size.y);
	scene->SetResolution(size);
}


void Engine::OnToggleCulling()
{
	culling = ! culling;
}

void Engine::OnToggleLighting()
{
	lighting = ! lighting;
}

void Engine::OnToggleWireframe()
{
	wireframe = ! wireframe;
}

void Engine::OnToggleCapture()
{
	capture_enable = ! capture_enable;
}

// TODO: Make camera an object so that positional messages are the same as for other objects.
void Engine::OnSetCamera(gfx::Transformation const & transformation)
{
	if (scene != nullptr)
	{
		scene->SetCameraTransformation(transformation);
	}

	// pass this on to the formation manager to update the node scores
	form::Daemon::Call([transformation] (form::Engine & engine) {
		engine.OnSetCamera(transformation);
	});
}

gfx::Transformation const& Engine::GetCamera() const
{
	return scene->GetPov().GetTransformation();
}

void Engine::OnSetRegulatorHandle(smp::Handle<form::RegulatorScript> regulator_handle)
{
	ASSERT(regulator_handle);
	_regulator_handle = regulator_handle;
}

#if defined(NDEBUG)
#define INIT(CAP,ENABLED) { CAP,ENABLED }
#else
#define INIT(CAP,ENABLED) { CAP,ENABLED,#CAP }
#endif

Engine::StateParam const Engine::init_state[] =
{
	INIT(GL_COLOR_MATERIAL, true),
	INIT(GL_TEXTURE_2D, false),
	INIT(GL_NORMALIZE, false),
	INIT(GL_CULL_FACE, true),
	INIT(GL_LIGHTING, false),
	INIT(GL_DEPTH_TEST, false),
	INIT(GL_BLEND, false),
	INIT(GL_INVALID_ENUM, false),
	INIT(GL_MULTISAMPLE, false),
	INIT(GL_LINE_SMOOTH, false),
	INIT(GL_POLYGON_SMOOTH, false),
	INIT(GL_FOG, false)
};

void Engine::Run(Daemon::MessageQueue & message_queue)
{
	while (! quit_flag)
	{
		ProcessMessagesAndGetReady(message_queue);
		VerifyObjectRef(* scene);
		
		PreRender();
		UpdateTransformations();
		Render();
		Capture();
	}
}

void Engine::ProcessMessagesAndGetReady(Daemon::MessageQueue & message_queue)
{
	// Process all pending messages.
	while (ProcessMessage(message_queue))
	{
	}
	
	// If we're mid-way through an uninterruptible set of messages,
	while (! _ready)
	{
		// keep processing
		if (! ProcessMessage(message_queue))
		{
			smp::Yield();
		}
	}
}

bool Engine::ProcessMessage(Daemon::MessageQueue & message_queue)
{
	return message_queue.DispatchMessage(* this);
}

bool Engine::Init()
{
	ASSERT(scene == nullptr);
	ASSERT(context == nullptr);
	SDL_Window & window = app::GetWindow();
	
	context = SDL_GL_CreateContext(& window);
	
	if (SDL_GL_MakeCurrent(& window, context) != 0)
	{
		DEBUG_BREAK_SDL();
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
	geom::Vector2i resolution = app::GetWindowSize();
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
//	geom::Vector2i resolution = app::GetWindowSize();
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
	init_lighting = lighting;
	init_wireframe = wireframe;
	
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

	SDL_GL_DeleteContext(context);
	context = nullptr;
}

// Decide whether to use vsync and initialize GL state accordingly.
void Engine::InitVSync()
{
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

	GL_CALL(glFrontFace(GL_CW));
	GL_CALL(glCullFace(GL_BACK));
	GL_CALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST));
	glDepthFunc(GL_LEQUAL);
	GL_CALL(glPolygonMode(GL_FRONT, GL_FILL));
	GL_CALL(glPolygonMode(GL_BACK, GL_FILL));
	GL_CALL(glClearDepth(1.0f));
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

	glMatrixMode(GL_MODELVIEW);
	
	GL_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
	GL_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));	// TODO: Broke!
	GL_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));
	
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
	ASSERT(GetInt<GL_DEPTH_FUNC>() == GL_LEQUAL);
	
	ASSERT(GetInt<GL_MATRIX_MODE>() == GL_MODELVIEW);
#endif	// NDEBUG
}

// TODO: Remove?
bool Engine::HasShadowSupport() const
{
	if (! enable_shadow_mapping) {
		return false;
	}
	
#if ! defined(__APPLE__)
	if (! GLEW_ARB_shadow) {
		return false;
	}
	if (! GLEW_ARB_depth_texture) {
		return false;
	}
	if (! GLEW_EXT_framebuffer_object) {
		return false;
	}
#endif
	
	return true;
}

void Engine::PreRender()
{
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
				scene->RemoveObject(leaf_node.GetUid());
				break;
		}
	}
}

void Engine::UpdateTransformations(BranchNode & parent_branch, gfx::Transformation const & model_view_transformation)
{
	VerifyObject(model_view_transformation);
	Transformation scratch;
	
	for (Object::List::iterator i = parent_branch.Begin(), end = parent_branch.End(); i != end; )
	{
		Object & child = * i;
		++ i;
		
		Transformation const & child_model_view_transformation = child.Transform(* this, model_view_transformation, scratch);
		VerifyObject(child_model_view_transformation);
		
		switch (child.GetNodeType())
		{
			default:
				ASSERT(false);
				
			case Object::branch:
			{
				BranchNode & child_branch = child.CastBranchNodeRef();
				if (child_branch.IsEmpty())
				{
					scene->RemoveObject(child_branch.GetUid());
					continue;
				}
				
				UpdateTransformations(child_branch, child_model_view_transformation);
				break;
			}
				
			case Object::leaf:
			{
				LeafNode & child_leaf = child.CastLeafNodeRef();
				LeafNode::Transformation transformation(child_model_view_transformation);
				child_leaf.SetModelViewTransformation(transformation);
				break;
			}
		}
	}
}

void Engine::UpdateTransformations()
{
	BranchNode & root_node = scene->GetRoot();
	Transformation const & root_transformation = root_node.GetTransformation();
	UpdateTransformations(root_node, root_transformation);
	
	scene->SortRenderList();
}

void Engine::Render()
{
	RenderScene();

	// Flip the front and back buffers and set fences.
	SetFence(_fence1);
	SDL_Window & window = app::GetWindow();
	SDL_GL_SwapWindow(& window);
	SetFence(_fence2);

	ProcessRenderTiming();
}

void Engine::RenderScene()
{
	VerifyRenderState();
	
	// The skybox, basically.
	RenderBackground();
	
	// All the things.
	RenderForeground();
	
	// Crap you generally don't want.
	DebugDraw();
	
	// Ensures that FormationMesh::PreRender functions correctly.
	SetCurrentMesh(nullptr);

	VerifyRenderState();
}

void Engine::RenderBackground()
{
	SetBackgroundFrustum(scene->GetPov());

	// basically draw the skybox
	int background_render_count = RenderLayer(Layer::background);
	
	// and if we have no skybox yet,
	if (background_render_count < 1)
	{
		// clear the screen.
		GL_CALL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	}
}

void Engine::RenderForeground()
{
	// Adjust near and far plane.
	SetForegroundFrustum(* scene);
	
	// Set up lights.
	RenderLights();
	
	// Draw material objects.
	if (wireframe)
	{
		RenderForegroundPass(WireframePass1);
		RenderForegroundPass(WireframePass2);
	}
	else 
	{
		RenderForegroundPass(NormalPass);
	}
}

void Engine::RenderLights()
{
	if (! lighting)
	{
		return;
	}
	
	for (int program_index = 0; program_index != ProgramIndex::max_shader; ++ program_index)
	{
		Program const * program = _resource_manager->GetProgram(static_cast<ProgramIndex::type>(program_index));
		program->OnLightsChanged();
	}
}

bool Engine::BeginRenderForeground(ForegroundRenderPass pass) const
{
	ASSERT(GetInt<GL_DEPTH_FUNC>() == GL_LEQUAL);
		
	switch (pass) 
	{
		case NormalPass:
			if (! culling) 
			{
				Disable(GL_CULL_FACE);
			}
			if (multisample)
			{
				Enable(GL_MULTISAMPLE);
				Enable(GL_POLYGON_SMOOTH);
			}
			break;
			
		case WireframePass1:
			if (! culling)
			{
				return false;
			}
			Enable(GL_POLYGON_OFFSET_FILL);
			GL_CALL(glPolygonOffset(1,1));
			glColor3f(1.f, 1.f, 1.f);
			break;
			
		case WireframePass2:
			if (! culling) 
			{
				Disable(GL_CULL_FACE);
			}
			if (multisample)
			{
				Enable(GL_MULTISAMPLE);
				Enable(GL_LINE_SMOOTH);
			}
			GL_CALL(glPolygonMode(GL_FRONT, GL_LINE));
			GL_CALL(glPolygonMode(GL_BACK, GL_LINE));
			glColor3f(0.f, 0.f, 0.f);
			break;
			
		default:
			ASSERT(false);
	}
	
	Enable(GL_DEPTH_TEST);
	
	return true;
}

// Each pass draws all the geometry. Typically, there is one per frame
// unless wireframe mode is on. 
void Engine::RenderForegroundPass(ForegroundRenderPass pass)
{
	// begin
	if (! BeginRenderForeground(pass))
	{
		return;
	}
	
	// render opaque objects
	RenderLayer(Layer::foreground);

	// render partially transparent objects
	Enable(GL_BLEND);
	glDepthMask(false);
	RenderLayer(Layer::foreground, false);
	Disable(GL_BLEND);
	glDepthMask(true);

	// end
	EndRenderForeground(pass);
}

void Engine::EndRenderForeground(ForegroundRenderPass pass) const
{
	// Reset state
	Disable(GL_DEPTH_TEST);

	switch (pass) 
	{
		case NormalPass:
			if (! culling) 
			{
				Enable(GL_CULL_FACE);
			}
			if (multisample)
			{
				Disable(GL_POLYGON_SMOOTH);
				Disable(GL_MULTISAMPLE);
			}
			break;
			
		case WireframePass1:
			GL_CALL(glCullFace(GL_BACK));
			Disable(GL_POLYGON_OFFSET_FILL);
			break;
			
		case WireframePass2:
			glColor3f(0.f, 0.f, 0.f);
			if (! culling) 
			{
				Enable(GL_CULL_FACE);
			}
			if (multisample)
			{
				Disable(GL_LINE_SMOOTH);
				Disable(GL_MULTISAMPLE);
			}
			GL_CALL(glPolygonMode(GL_FRONT, GL_FILL));
			GL_CALL(glPolygonMode(GL_BACK, GL_FILL));
			break;
			
		default:
			ASSERT(false);
	}
}

int Engine::RenderLayer(Layer::type layer, bool opaque)
{
	int num_rendered_objects = 0;
	
	typedef LeafNode::RenderList List;
	List & render_list = scene->GetRenderList();
	
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
		
		// Set the matrix.
		Transformation const & transformation = leaf_node.GetModelViewTransformation();
		SetModelViewMatrix(transformation);

		Program const * required_program = leaf_node.GetProgram();
		if (required_program != nullptr)
		{
			if (required_program->IsInitialized())
			{
				SetCurrentProgram(required_program);
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

void Engine::DebugDraw()
{
#if defined(GFX_DEBUG)
	if (capture_enable)
	{
		return;
	}

	Debug::AddBasis(sim::Vector3::Zero(), 1000000.);
	
	Pov const & pov = scene->GetPov();
	Transformation const & transformation = pov.GetTransformation();

	// Set the model view to the regular global view but at the origin
	Vector3 translation = transformation.GetTranslation();
	Matrix33 rotation = transformation.GetRotation();
	Matrix33 inverse_rotation = Inverse(rotation);
	Transformation model_view(Vector3::Zero(), inverse_rotation);
	SetModelViewMatrix(model_view);
	
	SetCurrentProgram(nullptr);
	SetCurrentMesh(nullptr);
	
	// then pass the missing translation into the draw function.
	// It corrects all the verts accordingly (avoids a precision issue).
	Debug::Draw(translation);
	
#if defined (GATHER_STATS)
	STAT_SET (pos, translation);	// std::streamsize previous_precision = out.precision(10); ...; out.precision(previous_precision);
	
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
	
	Debug::DrawText(out_stream.str().c_str(), geom::Vector2i::Zero());
#endif
	
#endif
}

void Engine::ProcessRenderTiming()
{
	Time frame_start_position, pre_sync_position, post_sync_position;
	GetRenderTiming(frame_start_position, pre_sync_position, post_sync_position);
	
	Time frame_duration, busy_duration;
	ConvertRenderTiming(frame_start_position, pre_sync_position, post_sync_position, frame_duration, busy_duration);
	
	UpdateFpsCounter(frame_start_position);
	
	UpdateRegulator(busy_duration);
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

void Engine::UpdateFpsCounter(Time frame_start_position)
{
#if ! defined(NDEBUG)
	// update the history
	memmove(_frame_time_history, _frame_time_history + 1, sizeof(* _frame_time_history) * (_frame_time_history_size - 1));
	Time first_entry = _frame_time_history[0];
	Time & last_entry = _frame_time_history[_frame_time_history_size - 1];
	last_entry = frame_start_position;
	
	// average it
	float history_duration = _frame_time_history_size - 1;
	float average_frame_duration = float(last_entry - first_entry) / history_duration;
	STAT_SET (fps, 1.f / average_frame_duration);
#endif
}

void Engine::UpdateRegulator(Time busy_duration) const
{
	// Regulator feedback.
	// TODO: There's no reason why frame rate should be tied to simulation tick rate.
	// TODO: Decouple these two, use frame-rate of video mode for this one 
	// and rename the other to something more appropriate.
	Time target_frame_duration = gfx_frame_duration;
	if (vsync)
	{
		target_frame_duration *= target_work_proportion;
	}
	
	float frame_duration_ratio = float(busy_duration / target_frame_duration);
	ASSERT(frame_duration_ratio >= 0);
	ASSERT(! IsInf(frame_duration_ratio));
	
	if (_regulator_handle)
	{
		//DEBUG_MESSAGE("bd:%f fdr:%f", busy_duration, frame_duration_ratio);
		_regulator_handle.Call([frame_duration_ratio] (form::RegulatorScript & script) {
			script.SampleFrameDuration(frame_duration_ratio);
		});
	}
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
		&& gfx::Image::CopyVFlip(capture_image[1], capture_image[0])
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

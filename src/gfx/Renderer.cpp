//
//  Renderer.cpp
//  crag
//
//  Created by John on 12/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Renderer.h"

#include "Color.h"
#include "Debug.h"
#include "Image.h"
#include "Pov.h"
#include "Scene.h"
#include "object/BranchNode.h"
#include "object/LeafNode.h"

#include "form/FormationManager.h"
#include "form/node/NodeBuffer.h"

#include "sim/axes.h"
#include "sim/Simulation.h"

#include "geom/MatrixOps.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/Statistics.h"

#include <sstream>


using namespace gfx;


CONFIG_DEFINE (multisample, bool, false);
CONFIG_DECLARE (profile_mode, bool);


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// File-local Variables
	
	//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
	CONFIG_DEFINE (background_ambient_color, Color4f, Color4f(0.1f));
	CONFIG_DEFINE (target_work_proportion, double, .95f);

	CONFIG_DEFINE (init_culling, bool, true);
	CONFIG_DEFINE (init_lighting, bool, true);
	CONFIG_DEFINE (init_wireframe, bool, false);

	CONFIG_DEFINE (enable_shadow_mapping, bool, true);
	
	CONFIG_DEFINE (capture_enable, bool, false);
	CONFIG_DEFINE (capture_skip, int, 1);
	
	// TODO
	//CONFIG_DEFINE (record_enable, bool, false);
	//CONFIG_DEFINE (record_playback, bool, false);
	//CONFIG_DEFINE (capture, bool, false);
	//CONFIG_DEFINE (record_playback_skip, int, 1);

	STAT (vsync_time, double, .18f);
	STAT (frame_duration, double, .18f);
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
	
	// Creates a frustum with sensible defaults for rendering a background.
	void SetBackgroundFrustum(Pov const & pov)
	{
		// Set projection matrix within relatively tight bounds.
		Frustum background_frustum = pov.GetFrustum();
		background_frustum.depth_range[0] = .1f;
		background_frustum.depth_range[1] = 10.f;
		background_frustum.SetProjectionMatrix();
	}
	
	// Given the scene (including frustum), creates an adjusted frustum
	// with suitable near/far z values and applies it to the projection matrix.
	void SetForegroundFrustum(Scene const & scene)
	{
		Pov const & pov = scene.GetPov();
		Frustum const & default_frustum = pov.GetFrustum();
		
		Frustum adjusted_frustum = default_frustum;
		adjusted_frustum.depth_range[0] = std::numeric_limits<float>::max();
		adjusted_frustum.depth_range[1] = - std::numeric_limits<float>::max();
		
		typedef LeafNode::RenderList List;
		List const & render_list = scene.GetRenderList();
		
		// For all leaf nodes in the render list,
		for (List::const_iterator i = render_list.begin(), end = render_list.end(); i != end; ++ i)
		{
			LeafNode const & leaf_node = * i;
			
			// if in the foreground layer,
			if (! leaf_node.IsInLayer(Layer::foreground))
			{
				continue;
			}
			
			// and has a render range,
			RenderRange depth_range;
			if (! leaf_node.GetRenderRange(depth_range))
			{
				continue;
			}
			
			// and it isn't behind the camera,
			if (depth_range[1] <= default_frustum.depth_range[0])
			{
				continue;
			}
			
			// then include it in adjusted frustum render range.
			if (depth_range[0] < adjusted_frustum.depth_range[0])
			{
				adjusted_frustum.depth_range[0] = depth_range[0];
			}
			
			if (depth_range[1] > adjusted_frustum.depth_range[1])
			{
				adjusted_frustum.depth_range[1] = depth_range[1];
			}
		}
		
		// Finally bind the near plane to the default frustum,
		adjusted_frustum.depth_range[0] = std::max(adjusted_frustum.depth_range[0], default_frustum.depth_range[0]);
		adjusted_frustum.SetProjectionMatrix();
	}

}	// namespace


Renderer::Renderer()
: context(nullptr)
, scene(nullptr)
, last_frame_time(app::GetTime())
, quit_flag(false)
, vsync(false)
, _ready(true)
, culling(init_culling)
, lighting(init_lighting)
, wireframe(init_wireframe)
, capture_frame(0)
{
#if ! defined(NDEBUG)
	std::fill(_fps_history, _fps_history + _fps_history_size, 0);
#endif
	
	if (! Init())
	{
		quit_flag = true;
	}
}

Renderer::~Renderer()
{
	Deinit();

	// must be turned on by key input or by cfg edit
	capture_enable = false;
}

Scene const & Renderer::GetScene() const
{
	Assert(Daemon::IsCurrentThread());
	
	return ref(scene);
}

void Renderer::OnQuit()
{
	quit_flag = true;
	_ready = true;
}

void Renderer::OnAddObject(Object * const & object, Uid const & parent_uid)
{
	if (! InitObject(* object))
	{
		delete object;
	}

	scene->AddObject(* object, parent_uid);
}

void Renderer::OnRemoveObject(Uid const & uid)
{
	if (scene != nullptr)
	{
		scene->RemoveObject(uid);
	}
}

void Renderer::OnSetReady(bool const & ready, Time const & time)
{
	_ready = ready;
	scene->SetTime(time);
}

void Renderer::OnResize(Vector2i const & size)
{
	gl::Viewport(0, 0, size.x, size.y);
	scene->SetResolution(size);
}


void Renderer::OnToggleCulling()
{
	culling = ! culling;
}

void Renderer::OnToggleLighting()
{
	lighting = ! lighting;
}

void Renderer::OnToggleWireframe()
{
	wireframe = ! wireframe;
}

void Renderer::OnToggleCapture()
{
	capture_enable = ! capture_enable;
}

// TODO: Make camera an object so that positional messages are the same as for other objects.
void Renderer::OnSetCamera(gfx::Transformation const & transformation)
{
	if (scene != nullptr)
	{
		scene->SetCameraTransformation(transformation);
	}

	// pass this on to the formation manager to update the node scores
	form::Daemon::Call(transformation, & form::FormationManager::OnSetCamera);
}

#if defined(NDEBUG)
#define INIT(CAP,ENABLED) { CAP,ENABLED }
#else
#define INIT(CAP,ENABLED) { CAP,ENABLED,#CAP }
#endif

Renderer::StateParam const Renderer::init_state[] =
{
	INIT(GL_COLOR_MATERIAL, true),
	INIT(GL_TEXTURE_2D, false),
	INIT(GL_NORMALIZE, true),
	INIT(GL_CULL_FACE, true),
	INIT(GL_LIGHTING, false),
	INIT(GL_DEPTH_TEST, false),
	INIT(GL_BLEND, false),
	INIT(GL_INVALID_ENUM, false),
	INIT(GL_MULTISAMPLE, false),
	INIT(GL_LINE_SMOOTH, false),
	INIT(GL_POLYGON_SMOOTH, false)
};

void Renderer::Run(Daemon::MessageQueue & message_queue)
{
	while (! quit_flag)
	{
		ProcessMessagesAndGetReady(message_queue);
		PreRender();
		UpdateTransformations();
		Render();
		Capture();
	}
}

void Renderer::ProcessMessagesAndGetReady(Daemon::MessageQueue & message_queue)
{
	if (vsync)
	{
		_ready = false;
	}
	
	int frames_to_skip;
	if (capture_enable)
	{
		frames_to_skip = capture_skip;
	}
	else 
	{
		frames_to_skip = 0;
		message_queue.DispatchMessages(* this);
	}
	
	for (int frame = 0; frame <= capture_skip; ++ frame)
	{
		while (! _ready)
		{
			if (! message_queue.DispatchMessage(* this))
			{
				smp::Yield();
			}
		}
	}
}

bool Renderer::Init()
{
	smp::SetThreadPriority(1);
	smp::SetThreadName("Renderer");
	
	Assert(scene == nullptr);
	Assert(context == nullptr);
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

	InitVSync();
	
	scene = new Scene;
	Vector2i resolution = app::GetWindowSize();
	scene->SetResolution(resolution);
	
	InitRenderState();
	
	Debug::Init();
	return true;
}

void Renderer::Deinit()
{
	if (scene == nullptr)
	{
		return;
	}

	Debug::Deinit();
	
	init_culling = culling;
	init_lighting = lighting;
	init_wireframe = wireframe;
	
	if (_fence2.IsInitialized())
	{
		gl::DeleteFence(_fence2);
	}

	if (_fence1.IsInitialized())
	{
		gl::DeleteFence(_fence1);
	}

	delete scene;

	SDL_GL_DeleteContext(context);
	context = nullptr;
}

bool Renderer::InitObject(Object & object)
{
	if (quit_flag || scene == nullptr)
	{
		// This is probably ok during shut-down but would like to see it happen.
		Assert(false);

		return false;
	}
	
	return object.Init(* this);
}

// Decide whether to use vsync and initialize GL state accordingly.
void Renderer::InitVSync()
{
	if (profile_mode)
	{
		vsync = false;
	}
	else
	{
		if (GlSupportsFences())
		{
			gl::GenFence(_fence1);
			gl::GenFence(_fence2);

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
			std::cerr << "Crag cannot run when vsync is forced on AND OpenGL feature, fences, are not supported." << std::endl;
			exit(1);

		default:
			Assert(false);
		}

		Assert(vsync);
	}

	// Something went wrong if we get to here.
	// We need to remove the fences and forgo vsync.

	vsync = false;
	if (_fence1.IsInitialized())
	{
		gl::DeleteFence(_fence1);
	}
	if (_fence2.IsInitialized())
	{
		gl::DeleteFence(_fence2);
	}
}

void Renderer::InitRenderState()
{
	StateParam const * param = init_state;
	while (param->cap != GL_INVALID_ENUM)
	{
		GLPP_CALL((param->enabled ? glEnable : glDisable)(param->cap));
		++ param;
	}

	GLPP_CALL(glFrontFace(GL_CW));
	GLPP_CALL(glCullFace(GL_BACK));
	GLPP_CALL(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST));
	gl::SetDepthFunc(GL_LEQUAL);
	GLPP_CALL(glPolygonMode(GL_FRONT, GL_FILL));
	GLPP_CALL(glPolygonMode(GL_BACK, GL_FILL));
	GLPP_CALL(glClearDepth(1.0f));
	gl::BlendFunc(gl::SRC_ALPHA, gl::ONE_MINUS_SRC_ALPHA);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

	gl::MatrixMode(GL_MODELVIEW);
	
	GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
	GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));	// TODO: Broke!
	GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));
	
	VerifyRenderState();
}

void Renderer::VerifyRenderState() const
{
#if ! defined(NDEBUG)
	StateParam const * param = init_state;
	while (param->cap != GL_INVALID_ENUM)
	{
#if ! defined(NDEBUG)
		if (param->enabled != gl::IsEnabled(param->cap))
		{
			std::cerr << "Bad render state: " << param->name << std::endl;
			Assert(false);
		}
#endif
		++ param;
	}
	
	// TODO: Write equivalent functions for all state in GL. :S
	Assert(gl::GetDepthFunc() == GL_LEQUAL);
	
	Assert(gl::GetMatrixMode() == GL_MODELVIEW);
#endif	// NDEBUG
}

// TODO: Remove?
bool Renderer::HasShadowSupport() const
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

void Renderer::PreRender()
{
	typedef LeafNode::RenderList List;
	
	List const & render_list = scene->GetRenderList();
	for (List::iterator i = render_list.begin(), end = render_list.end(); i != end; )
	{
		LeafNode & leaf_node = * i;
		++ i;
		
		LeafNode::PreRenderResult result = leaf_node.PreRender(* this);
		
		switch (result)
		{
			default:
				Assert(false);
			case LeafNode::ok:
				break;
			case LeafNode::remove:
				scene->RemoveObject(leaf_node.GetUid());
				break;
		}
	}
}

void Renderer::UpdateTransformations(BranchNode & parent_branch, gfx::Transformation const & model_view_transformation)
{
	Transformation scratch;
	
	for (ObjectBase::ChildList::iterator i = parent_branch.Begin(), end = parent_branch.End(); i != end; )
	{
		Object & child = static_cast<Object &>(* i);
		++ i;
		
		Transformation const & child_model_view_transformation = child.Transform(model_view_transformation, scratch, scene->GetTime());
		
		switch (child.GetNodeType())
		{
			default:
				Assert(false);
				
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

void Renderer::UpdateTransformations()
{
	BranchNode & root_node = scene->GetRoot();
	Transformation const & root_transformation = root_node.GetTransformation();
	UpdateTransformations(root_node, root_transformation);
	
	scene->SortRenderList();
}

void Renderer::Render()
{
	RenderScene();

	// Flip the front and back buffers and set fences.
	SetFence(_fence1);
	SDL_Window & window = app::GetWindow();
	SDL_GL_SwapWindow(& window);
	SetFence(_fence2);

	ProcessRenderTiming();
}

void Renderer::RenderScene() const
{
	VerifyRenderState();
	
	// The skybox, basically.
	RenderBackground();
	
	// All the things.
	RenderForeground();
	
	// Crap you generally don't want.
	DebugDraw();

	VerifyRenderState();
}

void Renderer::RenderBackground() const
{
	SetBackgroundFrustum(scene->GetPov());

	// basically draw the skybox
	int background_render_count = RenderLayer(Layer::background);
	
	// and if we have no skybox yet,
	if (background_render_count < 1)
	{
		// clear the screen.
		GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	}
}

void Renderer::RenderForeground() const
{
	// Adjust near and far plane.
	SetForegroundFrustum(* scene);
	
	// Set up lights.
	if (lighting)
	{
		gl::Enable(GL_LIGHTING);
		
		RenderLayer(Layer::light);
	}
	
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

bool Renderer::BeginRenderForeground(ForegroundRenderPass pass) const
{
	Assert(gl::GetDepthFunc() == GL_LEQUAL);
		
	switch (pass) 
	{
		case NormalPass:
			if (! culling) 
			{
				gl::Disable(GL_CULL_FACE);
			}
			if (multisample)
			{
				gl::Enable(GL_MULTISAMPLE);
				gl::Enable(GL_POLYGON_SMOOTH);
			}
			break;
			
		case WireframePass1:
			if (! culling)
			{
				return false;
			}
			gl::Enable(GL_POLYGON_OFFSET_FILL);
			GLPP_CALL(glPolygonOffset(1,1));
			gl::SetColor<GLfloat>(1.f, 1.f, 1.f);
			break;
			
		case WireframePass2:
			if (! culling) 
			{
				gl::Disable(GL_CULL_FACE);
			}
			if (multisample)
			{
				gl::Enable(GL_MULTISAMPLE);
				gl::Enable(GL_LINE_SMOOTH);
			}
			GLPP_CALL(glPolygonMode(GL_FRONT, GL_LINE));
			GLPP_CALL(glPolygonMode(GL_BACK, GL_LINE));
			gl::SetColor<GLfloat>(0.f, 0.f, 0.f);
			break;
			
		default:
			Assert(false);
	}
	
	gl::Enable(GL_DEPTH_TEST);
	
	return true;
}

// Each pass draws all the geometry. Typically, there is one per frame
// unless wireframe mode is on. 
void Renderer::RenderForegroundPass(ForegroundRenderPass pass) const
{
	// begin
	if (! BeginRenderForeground(pass))
	{
		return;
	}
	
	// render opaque objects
	RenderLayer(Layer::foreground);

	// render partially transparent objects
	gl::Enable(GL_BLEND);
	gl::SetDepthMask(false);
	RenderLayer(Layer::foreground, false);
	gl::Disable(GL_BLEND);
	gl::SetDepthMask(true);

	// end
	EndRenderForeground(pass);
}

void Renderer::EndRenderForeground(ForegroundRenderPass pass) const
{
	// Reset state
	if (lighting && pass != WireframePass2)
	{
		gl::Disable(GL_LIGHTING);
	}
	gl::Disable(GL_DEPTH_TEST);

	switch (pass) 
	{
		case NormalPass:
			if (! culling) 
			{
				gl::Enable(GL_CULL_FACE);
			}
			if (multisample)
			{
				gl::Disable(GL_POLYGON_SMOOTH);
				gl::Disable(GL_MULTISAMPLE);
			}
			break;
			
		case WireframePass1:
			GLPP_CALL(glCullFace(GL_BACK));
			gl::Disable(GL_POLYGON_OFFSET_FILL);
			break;
			
		case WireframePass2:
			gl::SetColor<GLfloat>(0.f, 0.f, 0.f);
			if (! culling) 
			{
				gl::Enable(GL_CULL_FACE);
			}
			if (multisample)
			{
				gl::Disable(GL_LINE_SMOOTH);
				gl::Disable(GL_MULTISAMPLE);
			}
			GLPP_CALL(glPolygonMode(GL_FRONT, GL_FILL));
			GLPP_CALL(glPolygonMode(GL_BACK, GL_FILL));
			break;
			
		default:
			Assert(false);
	}
}

int Renderer::RenderLayer(Layer::type layer, bool opaque) const
{
	int num_rendered_objects = 0;
	
	typedef LeafNode::RenderList List;
	List const & render_list = scene->GetRenderList();
	
	for (List::const_iterator i = render_list.begin(), end = render_list.end(); i != end; ++ i)
	{
		LeafNode const & leaf_node = * i;
		
		if (leaf_node.IsInLayer(layer) && leaf_node.IsOpaque() == opaque)
		{
			// Set the matrix.
			Transformation const & transformation = leaf_node.GetModelViewTransformation();
			Pov::SetModelViewMatrix(transformation);

			leaf_node.Render(* this);
			++ num_rendered_objects;
		}
	}
	
	return num_rendered_objects;
}

void Renderer::DebugDraw() const
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
	Pov::SetModelViewMatrix(model_view);
	
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
	
	Debug::DrawText(out_stream.str().c_str(), Vector2i::Zero());
#endif
	
#endif
}

void Renderer::ProcessRenderTiming()
{
	Time pre_sync, post_sync;

	if (vsync)
	{
		// Get timing information from the fences.
		FinishFence(_fence1);
		pre_sync = app::GetTime();
		FinishFence(_fence2);
		post_sync = app::GetTime();
	}
	else
	{
		// There is no sync.
		pre_sync = post_sync = app::GetTime();
	}
	
	Assert(last_frame_time <= pre_sync);
	Assert(pre_sync <= post_sync);
	
	Time vsync_time = post_sync - pre_sync;
	STAT_SET(vsync_time, vsync_time);

	// Use it to figure out just how much work was done.
	Time frame_time = post_sync;
	Time frame_duration = frame_time - last_frame_time;
	last_frame_time = frame_time;
	STAT_SET(frame_duration, frame_duration);

	Time busy_time = frame_duration - vsync_time;
	
#if ! defined(NDEBUG)
	// update fps
	memmove(_fps_history, _fps_history + 1, sizeof(* _fps_history) * (_fps_history_size - 1));
	_fps_history[_fps_history_size - 1] = frame_time;
	STAT_SET (fps, float(_fps_history_size - 1) / float(_fps_history[_fps_history_size - 1] - _fps_history[0]));
#endif
	
	// Regulator feedback.
	Time target_frame_duration = sim::Simulation::target_frame_seconds * 2;
	if (vsync)
	{
		target_frame_duration *= target_work_proportion;
	}

	float fitness = float(target_frame_duration / busy_time);
	Assert(fitness >= 0);
	
	form::Daemon::Call(fitness, & form::FormationManager::OnRegulatorSetFrame);
}

void Renderer::Capture()
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

void Renderer::SetFence(gl::Fence & fence)
{
	if (fence.IsInitialized())
	{
		gl::SetFence(fence);
	}
}

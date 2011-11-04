/*
 *  Renderer.cpp
 *  Crag
 *
 *  Created by John on 12/19/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Renderer.h"

#include "Color.h"
#include "Debug.h"
#include "Image.h"
#include "Pov.h"
#include "Scene.h"
#include "object/Object.h"

#include "form/FormationManager.h"
#include "form/node/NodeBuffer.h"

#include "sim/axes.h"
#include "sim/Simulation.h"

#include "core/ConfigEntry.h"
#include "core/Statistics.h"

#include <sstream>


using sys::TimeType;
using namespace gfx;


CONFIG_DEFINE (multisample, bool, false);
CONFIG_DECLARE (profile_mode, bool);


namespace 
{

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
	
	// Given the scene (including frustum), creates an adjusted frustum
	// with suitable near/far z values and applies it to the projection matrix.
	void SetForegroundFrustum(Scene const & scene, bool wireframe)
	{
		Pov const & pov = scene.GetPov();
		Frustum const & pov_frustum = pov.GetFrustum();
		Frustum adjusted_frustum = pov_frustum;
		adjusted_frustum.near_z = std::numeric_limits<float>::max();

		sim::Ray3 camera_ray = axes::GetCameraRay(pov.GetTransformation());
		
		scene.GetRenderRange(camera_ray, adjusted_frustum.near_z, adjusted_frustum.far_z, wireframe);
		
		adjusted_frustum.near_z = Max(adjusted_frustum.near_z * .5, pov_frustum.near_z);
		adjusted_frustum.SetProjectionMatrix();
	}
	
}	// namespace


Renderer::Renderer()
: last_frame_time(sys::GetTime())
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
}

void Renderer::OnQuit()
{
	quit_flag = true;
	_ready = true;
}

void Renderer::OnAddObject(Object * const & object)
{
	if (quit_flag)
	{
		return;
	}

	object->Init();

	if (scene != nullptr)
	{
		scene->AddObject(* object);
	}
}

void Renderer::OnRemoveObject(Object * const & object)
{
	if (scene != nullptr)
	{
		scene->RemoveObject(* object);
		object->Deinit();
	}
	delete object;
}

void Renderer::OnSetReady(bool const & ready)
{
	_ready = ready;
}

void Renderer::OnResize(Vector2i const & size)
{
	scene->SetResolution(size);

	form::Daemon::Call(& form::FormationManager::OnRegulatorReset);
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
void Renderer::OnSetCamera(Transformation<double> const & transformation)
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
	INIT(GL_LIGHT0, false),
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
	
	if (! sys::GlInit())
	{
		scene = nullptr;
		return false;
	}

	InitVSync();
	
	scene = new Scene;
	scene->SetResolution(sys::GetWindowSize());
	
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

	sys::GlDeinit();
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
		if (sys::GlSupportsFences())
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

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

	gl::MatrixMode(GL_MODELVIEW);
	
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
	ObjectSet & objects = scene->GetObjects(Layer::pre_render);
	for (ObjectSet::iterator i = objects.begin(); i != objects.end(); ++ i)
	{
		Object & object = ref(* i);
		
		Assert(object.IsInLayer(Layer::pre_render));
		
		object.PreRender();
	}
}

void Renderer::Render()
{
	RenderScene();

	// Flip the front and back buffers and set fences.
	SetFence(_fence1);
	sys::SwapBuffers();
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
	ObjectSet const & objects = scene->GetObjects(Layer::background);

	// Basically, if we have a skybox yet,
	if (objects.size() > 0)
	{
		// only clear the depth buffer
		GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT));
		
		// and draw the skybox
		RenderLayer(Layer::background);
	}
	else
	{
		// else, clear the screen to black.
		GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	}
}

void Renderer::RenderForeground() const
{
	// Adjust near and far plane
	SetForegroundFrustum(* scene, wireframe && false);
	
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
	
	// Set state
	if (lighting && pass != WireframePass2)
	{
		gl::Enable(GL_LIGHTING);
		EnableLights(true);
	}
	
	gl::Enable(GL_DEPTH_TEST);
	
	return true;
}

// Each pass draws all the geometry. Typically, there is one per frame
// unless wireframe mode is on. 
void Renderer::RenderForegroundPass(ForegroundRenderPass pass) const
{
	if (! BeginRenderForeground(pass))
	{
		return;
	}
	
	RenderLayer(Layer::foreground);
	
	EndRenderForeground(pass);
	
	VerifyRenderState();
}

void Renderer::EndRenderForeground(ForegroundRenderPass pass) const
{
	// Reset state
	if (lighting && pass != WireframePass2)
	{
		EnableLights(false);
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
			gl::SetColor<GLfloat>(1, 1, 1);
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
	gl::SetDepthFunc(GL_LEQUAL);
}

// Nothing to do with shadow maps; sets/unsets the lights in the main scene.
void Renderer::EnableLights(bool enabled) const
{
	if (enabled)
	{
		// TODO: Move this out of Render routine.
		GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
		GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));	// TODO: Broke!
		GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));
	}
	
	ObjectSet lights = scene->GetObjects(Layer::light);
	int light_id = GL_LIGHT0; 
	for (ObjectSet::const_iterator it = lights.begin(); it != lights.end(); ++ it) 
	{
		Object const & light = ref(* it);
		Assert(light.IsInLayer(Layer::light));
		
		if (enabled)
		{
			gl::Enable(light_id);
			light.Render(Layer::light, * scene);
		}
		else
		{
			gl::Disable(light_id);
		}
		
		++ light_id;
		if (light_id > GL_LIGHT7) {
			Assert(false);	// too many lights
			break;
		}
	}
	
	while (light_id <= GL_LIGHT7) {
		Assert(! gl::IsEnabled(light_id));
		++ light_id;
	}
}

void Renderer::RenderLayer(Layer::type layer) const
{
	ObjectSet const & objects = scene->GetObjects(layer);
	for (ObjectSet::const_iterator i = objects.begin(); i != objects.end(); ++ i)
	{
		Object const & object = ref(* i);
		
		Assert(object.IsInLayer(layer));
		
		object.Render(layer, * scene);
	}
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
	Pov::Transformation const & transformation = pov.GetTransformation();
	sim::Vector3 pos = transformation.GetTranslation();
	pov.SetModelView(Pov::Transformation(pos));
	Debug::Draw(pos);
	
#if defined (GATHER_STATS)
	STAT_SET (pos, pos);	// std::streamsize previous_precision = out.precision(10); ...; out.precision(previous_precision);
	
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
	TimeType pre_sync, post_sync;

	if (vsync)
	{
		// Get timing information from the fences.
		FinishFence(_fence1);
		pre_sync = sys::GetTime();
		FinishFence(_fence2);
		post_sync = sys::GetTime();
	}
	else
	{
		// There is no sync.
		pre_sync = post_sync = sys::GetTime();
	}
	TimeType vsync_time = post_sync - pre_sync;
	STAT_SET(vsync_time, vsync_time);

	// Use it to figure out just how much work was done.
	TimeType frame_time = post_sync;
	TimeType frame_duration = frame_time - last_frame_time;
	last_frame_time = frame_time;
	STAT_SET(frame_duration, frame_duration);

	TimeType busy_time = frame_duration - vsync_time;
	
#if ! defined(NDEBUG)
	// update fps
	memmove(_fps_history, _fps_history + 1, sizeof(* _fps_history) * (_fps_history_size - 1));
	_fps_history[_fps_history_size - 1] = frame_time;
	STAT_SET (fps, float(_fps_history_size - 1) / float(_fps_history[_fps_history_size - 1] - _fps_history[0]));
#endif
	
	// Regulator feedback.
	TimeType target_frame_duration = sim::Simulation::target_frame_seconds * 2;
	if (vsync)
	{
		target_frame_duration *= target_work_proportion;
	}

	float fitness = float(target_frame_duration / busy_time);
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


Daemon * Renderer::singleton = nullptr;

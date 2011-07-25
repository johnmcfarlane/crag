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
#include "Pov.h"
#include "Scene.h"
#include "Skybox.h"
#include "Light.h"

#include "form/FormationManager.h"

#include "sim/axes.h"

#include "glpp/Texture.h"

#include "core/ConfigEntry.h"
#include "core/Statistics.h"

#include "sim/Entity.h"
#include "sim/Firmament.h"	// TODO
#include "sim/Simulation.h"

#include <sstream>


#if ! defined(NDEBUG)
extern gl::TextureRgba8 const * test_texture;
#endif


namespace 
{

	//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
	CONFIG_DEFINE (background_ambient_color, gfx::Color4f, gfx::Color4f(0.1f));
	CONFIG_DEFINE (target_work_proportion, double, .95f);

	CONFIG_DEFINE (init_culling, bool, true);
	CONFIG_DEFINE (init_lighting, bool, true);
	CONFIG_DEFINE (init_wireframe, bool, true);

	CONFIG_DEFINE (enable_shadow_mapping, bool, true);

	STAT (idle, double, .18f);
	STAT (fps, float, .0f);
	STAT (pos, sim::Vector3, .3f);
	STAT_DEFAULT (rot, sim::Matrix4, .9f);
	
	void SetForegroundFrustum(gfx::Scene const & scene, bool wireframe)
	{
		gfx::Pov const & pov = scene.GetPov();
		gfx::Frustum frustum = pov.frustum;
		frustum.near_z = std::numeric_limits<float>::max();

		sim::Ray3 camera_ray = axes::GetCameraRay(pov.pos, pov.rot);
		
		scene.GetRenderRange(camera_ray, frustum.near_z, frustum.far_z, wireframe);
		
		frustum.near_z = Max(frustum.near_z * .5, pov.frustum.near_z);
		frustum.SetProjectionMatrix();
	}
	
}	// namespace


gfx::Renderer::Renderer()
: last_frame_time(sys::GetTime())
, quit_flag(false)
, culling(init_culling)
, lighting(init_lighting)
, wireframe(init_wireframe)
, render_flag(false)
, average_frame_time(0)
, frame_count(0)
, frame_count_reset_time(last_frame_time)
{
	Assert(singleton == nullptr);
	singleton = this;
}

gfx::Renderer::~Renderer()
{
	Assert(singleton == this);
	singleton = nullptr;
}

void gfx::Renderer::OnMessage(smp::TerminateMessage const & message)
{
	quit_flag = true;
}

void gfx::Renderer::OnMessage(AddObjectMessage const & message)
{
	Object & object = message._object;
	scene->AddObject(object);
}

void gfx::Renderer::OnMessage(RemoveObjectMessage const & message)
{
	Object & object = message._object;
	scene->RemoveObject(object);
	delete & object;
}

void gfx::Renderer::OnMessage(RenderMessage const & message)
{
	//render_flag = true;
	++ render_flag;
}

void gfx::Renderer::OnMessage(ResizeMessage const & message)
{
	scene->SetResolution(message.size);
	form::FormationManager::Ref().ResetRegulator();
}

// TODO: Make camera an object so that positional messages are the same as for other objects.
void gfx::Renderer::OnMessage(sim::SetCameraMessage const & message)
{
	scene->SetCamera(message.projection.pos, message.projection.rot);
}

#if defined(NDEBUG)
#define INIT(CAP,ENABLED) { CAP,ENABLED }
#else
#define INIT(CAP,ENABLED) { CAP,ENABLED,#CAP }
#endif

gfx::Renderer::StateParam const gfx::Renderer::init_state[] =
{
	INIT(GL_COLOR_MATERIAL, true),
	INIT(GL_TEXTURE_2D, false),
	INIT(GL_NORMALIZE, true),
	INIT(GL_CULL_FACE, true),
	INIT(GL_LIGHT0, false),
	INIT(GL_LIGHTING, false),
	INIT(GL_DEPTH_TEST, false),
	INIT(GL_BLEND, false),
	INIT(GL_INVALID_ENUM, false)
};

void gfx::Renderer::Run()
{
	FUNCTION_NO_REENTRY;
	
	smp::SetThreadPriority(1);
	smp::SetThreadName("Renderer");
	
	sys::MakeCurrent();
	
	scene = new Scene;
	scene->SetResolution(sys::GetWindowSize());
	
	Object & skybox = ref(new Firmament);
	scene->AddObject(skybox);	// TODO: Call still belongs in sim
	
	InitRenderState();
	
	Debug::Init();
	
	MainLoop();

	Debug::Deinit();
	
	init_culling = culling;
	init_lighting = lighting;
	init_wireframe = wireframe;
	
	scene->RemoveObject(skybox);
	delete & skybox;
	
	delete scene;
}

void gfx::Renderer::MainLoop()
{
	form::FormationManager & formation_manager = form::FormationManager::Ref();
	
	bool busy = false;
	do
	{
		if (render_flag)
		{
			Render();
			
			busy = true;
			
			RendererReadyMessage message;
			sim::Simulation::SendMessage(message);

//			do
//			{
//				render_flag = false;
//			}	while (ProcessMessage());
		}
		
		if (formation_manager.PollMesh())
		{
			busy = true;
		}

		if (! busy)
		{
			smp::Sleep(0);
			busy = false;
		}

		if (ProcessMessages())
		{
			busy = true;
		}
	}
	while (! quit_flag);
}

void gfx::Renderer::InitRenderState()
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
	GLPP_CALL(gl::SetColor(gfx::Color4f::White().GetArray()));

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	VerifyRenderState();
}

void gfx::Renderer::VerifyRenderState() const
{
#if ! defined(NDEBUG)
	StateParam const * param = init_state;
	while (param->cap != GL_INVALID_ENUM)
	{
#if ! defined(NDEBUG)
		if (param->enabled != gl::IsEnabled(param->cap))
		{
			std::cout << "Bad render state: " << param->name << std::endl;
			Assert(false);
		}
#endif
		++ param;
	}
	
	// TODO: Write equivalent functions for all state in GL. :S
	Assert(gl::GetDepthFunc() == GL_LEQUAL);
	
	gfx::Color4f rgba;
	gl::GetColor(rgba.GetArray());
	Assert(rgba == gfx::Color4f::White());
#endif	// NDEBUG
}

// TODO: Remove?
bool gfx::Renderer::HasShadowSupport() const
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

void gfx::Renderer::ToggleLighting()
{
	lighting = ! lighting;
}

void gfx::Renderer::ToggleCulling()
{
	culling = ! culling;
}

void gfx::Renderer::ToggleWireframe()
{
	wireframe = ! wireframe;
}

void gfx::Renderer::Render()
{
	// Render the scene to the back buffer.
	RenderScene();	

	sys::TimeType frame_time;
	
	// A guestimate of the time spent waiting for vertical synchronization. 
	sys::TimeType idle;
	
	// Flip the front and back buffers and get timing information.

	// I have no idea why this works, or seems to work
	// or works on my hardware, or even seems to work on my hardware
	// but it often does. I'm especially baffled by the glFlush().
	
	glFlush();
	sys::TimeType pre_finish = sys::GetTime();
	sys::SwapBuffers();
	glFinish();
	frame_time = sys::GetTime();
	idle = frame_time - pre_finish;
	
	STAT_SET(idle, idle);

//	// Independant stat counters for measuring the FPS.
//	++ frame_count;
//	sys::TimeType fps_delta = frame_time - frame_count_reset_time;
//	if (frame_count == 60) 
//	{
//		frame_count_reset_time = frame_time;
//		average_frame_time = static_cast<float>(fps_delta) / frame_count;
//		frame_count = 0;
//	}
	
	// Calculate the amount of time from the last frame to this one.
//	sys::TimeType frame_delta = frame_time - last_frame_time;
//	Assert(frame_delta >= idle);
//	last_frame_time = frame_time;

	// Regulator feedback.
//	sys::TimeType frame_time = frame_delta - idle * .5;
//	sys::TimeType target_frame_time = sim::Simulation::target_frame_seconds;
//	if (enable_vsync)
//	{
//		target_frame_time *= target_work_proportion;
//	}
	
	//formation_manager.SampleFrameRatio(frame_time, target_frame_time);
}

void gfx::Renderer::RenderScene() const
{
	VerifyRenderState();

	// The skybox, basically.
	// TODO: Why not do this last?
	// TODO: And for that matter, formations second to last.
	RenderBackground();
	
	// All the things.
	RenderForeground();
	
	// Crap you generally don't want.
	DebugDraw();

	VerifyRenderState();
}

void gfx::Renderer::RenderBackground() const
{
	GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT));
	Draw(RenderStage::background);
}

void gfx::Renderer::RenderForeground() const
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

bool gfx::Renderer::BeginRenderForeground(ForegroundRenderPass pass) const
{
	Assert(gl::GetDepthFunc() == GL_LEQUAL);
	//gl::SetDepthFunc(GL_GREATER);
		
	switch (pass) 
	{
		case NormalPass:
			if (! culling) 
			{
				gl::Disable(GL_CULL_FACE);
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
			if (! culling) {
				gl::Disable(GL_CULL_FACE);
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
void gfx::Renderer::RenderForegroundPass(ForegroundRenderPass pass) const
{
	if (! BeginRenderForeground(pass))
	{
		return;
	}
	
	// Render the formations.
	form::FormationManager & fm = form::FormationManager::Ref();
	fm.Render(scene->GetPov());
	
	Draw(RenderStage::foreground);
	
	EndRenderForeground(pass);
	
	VerifyRenderState();
}

void gfx::Renderer::EndRenderForeground(ForegroundRenderPass pass) const
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
			if (! culling) {
				gl::Enable(GL_CULL_FACE);
			}
			break;
			
		case WireframePass1:
			GLPP_CALL(glCullFace(GL_BACK));
			gl::Disable(GL_POLYGON_OFFSET_FILL);
			break;
			
		case WireframePass2:
			gl::SetColor<GLfloat>(1, 1, 1);
			if (! culling) {
				gl::Enable(GL_CULL_FACE);
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
void gfx::Renderer::EnableLights(bool enabled) const
{
	if (enabled)
	{
		// TODO: Move this out of Render routine.
		GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
		GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));	// TODO: Broke!
		GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));
	}
	
	ObjectVector lights = scene->GetObjects(RenderStage::light);
	int light_id = GL_LIGHT0; 
	for (ObjectVector::const_iterator it = lights.begin(); it != lights.end(); ++ it) 
	{
		Object const & light = ref(* it);
		
		if (enabled)
		{
			gl::Enable(light_id);
			light.Draw(* scene);
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

void gfx::Renderer::Draw(RenderStage::type render_stage) const
{
	ObjectVector const & objects = scene->GetObjects(render_stage);
	
	for (ObjectVector::const_iterator i = objects.begin(); i != objects.end(); ++ i)
	{
		Object const & object = ref(* i);
		object.Draw(* scene);
	}
}

void gfx::Renderer::DebugDraw() const
{
#if defined(GFX_DEBUG)
	Pov const & pov = scene->GetPov();

	//Pov test = pov;
	//test.frustum.resolution = Vector2i(100, 100);
	//test.LookAtSphere(sim::Vector3(0, -200000.f, 0), sim::Sphere3(sim::Vector3::Zero(), 199999.f), Space::GetUp<float>());
	//Debug::AddFrustum(test);

	//sim::Vector3 forward = axes::GetForward(pov.rot);
	//sim::Vector3 ahead = pov.pos + (forward * (sim::Scalar)(pov.frustum.near_z + 1));
	//Debug::DrawLine(ahead, ahead + sim::Vector3(1, 1, 1));
	
	Debug::Draw();
	
#if defined (GATHER_STATS)
	STAT_SET (fps, 1.f / average_frame_time);
	STAT_SET (pos, pov.pos);	// std::streamsize previous_precision = out.precision(10); ...; out.precision(previous_precision);
	STAT_SET (rot, pov.rot);	// std::streamsize previous_precision = out.precision(2); ...; out.precision(previous_precision);
	
	// The string into which is written gfx::Debug text.
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


gfx::Renderer * gfx::Renderer::singleton = nullptr;

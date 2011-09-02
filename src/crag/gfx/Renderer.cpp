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

namespace 
{

	//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
	CONFIG_DEFINE (background_ambient_color, Color4f, Color4f(0.1f));
	CONFIG_DEFINE (target_work_proportion, double, .95f);

	CONFIG_DEFINE (init_culling, bool, true);
	CONFIG_DEFINE (init_lighting, bool, true);
	CONFIG_DEFINE (init_wireframe, bool, true);

	CONFIG_DEFINE (enable_shadow_mapping, bool, true);

	STAT (idle, double, .18f);
	STAT (frame_time, double, .18f);
	STAT (fps, float, .0f);
	STAT (pos, sim::Vector3, .3f);
	STAT_DEFAULT (rot, sim::Matrix4, .9f);
	
	void SetForegroundFrustum(Scene const & scene, bool wireframe)
	{
		Pov const & pov = scene.GetPov();
		Frustum frustum = pov.frustum;
		frustum.near_z = std::numeric_limits<float>::max();

		sim::Ray3 camera_ray = axes::GetCameraRay(pov.pos, pov.rot);
		
		scene.GetRenderRange(camera_ray, frustum.near_z, frustum.far_z, wireframe);
		
		frustum.near_z = Max(frustum.near_z * .5, pov.frustum.near_z);
		frustum.SetProjectionMatrix();
	}
	
}	// namespace


Renderer::Renderer()
: last_frame_time(sys::GetTime())
, quit_flag(false)
, culling(init_culling)
, lighting(init_lighting)
, wireframe(init_wireframe)
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

void Renderer::OnMessage(smp::TerminateMessage const & message)
{
	quit_flag = true;
}

void Renderer::OnMessage(AddObjectMessage const & message)
{
	Object & object = message._object;
	object.Init();
	scene->AddObject(object);
}

void Renderer::OnMessage(RemoveObjectMessage const & message)
{
	Object & object = message._object;
	scene->RemoveObject(object);
	object.Deinit();
	delete & object;
}

void Renderer::OnMessage(RenderReadyMessage const & message)
{
	ready = message.ready;
}

void Renderer::OnMessage(ResizeMessage const & message)
{
	scene->SetResolution(message.size);
	form::FormationManager::Daemon::Ref().ResetRegulator();
}

// TODO: Make camera an object so that positional messages are the same as for other objects.
void Renderer::OnMessage(sim::SetCameraMessage const & message)
{
	scene->SetCamera(message.projection.pos, message.projection.rot);
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
		Render();
	}
}

void Renderer::ProcessMessagesAndGetReady(Daemon::MessageQueue & message_queue)
{
	message_queue.DispatchMessages(* this);
	while (! ready)
	{
		if (! message_queue.DispatchMessage(* this))
		{
			smp::Yield();
		}
	}
}

bool Renderer::Init()
{
	smp::SetThreadPriority(1);
	smp::SetThreadName("Renderer");
	
	if (! sys::InitGl())
	{
		return false;
	}
	
	scene = new Scene;
	scene->SetResolution(sys::GetWindowSize());
	
	for (int index = 0; index < 2; ++ index)
	{
		// initialize mesh buffer
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Init();
		mbo.Bind();
		mbo.Resize(form::NodeBuffer::max_num_verts, form::NodeBuffer::max_num_indices, gl::DYNAMIC_DRAW);
		mbo.Unbind();
	}
	
	gl::GenFence(_fence1);
	gl::GenFence(_fence2);
	
	InitRenderState();
	
	Debug::Init();
	return true;
}

void Renderer::Deinit()
{
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

	for (int index = 0; index < 2; ++ index)
	{
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Deinit();
	}
	
	delete scene;

	sys::DeinitGl();
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
	GLPP_CALL(gl::SetColor(Color4f::White().GetArray()));

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);

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
	
	Color4f rgba;
	gl::GetColor(rgba.GetArray());
	Assert(rgba == Color4f::White());
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

void Renderer::ToggleLighting()
{
	lighting = ! lighting;
}

void Renderer::ToggleCulling()
{
	culling = ! culling;
}

void Renderer::ToggleWireframe()
{
	wireframe = ! wireframe;
}

void Renderer::Render()
{
	// Render the scene to the back buffer.
	form::FormationManager & formation_manager = form::FormationManager::Daemon::Ref();	
	form::MeshBufferObject & back_mbo = mbo_buffers.back();
	if (formation_manager.PollMesh(back_mbo))
	{
		mbo_buffers.flip();
	}

	RenderScene();

	// Flip the front and back buffers and set fences.
	SetFence(_fence1);
	sys::SwapBuffers();
	SetFence(_fence2);
	
	// Get timing information from the fences.
	FinishFence(_fence1);
	TimeType pre_sync = sys::GetTime();
	FinishFence(_fence2);
	TimeType post_sync = sys::GetTime();
	TimeType idle = post_sync - pre_sync;
	STAT_SET(idle, idle);
	
	// Use it to figure out just how much work was done.
	TimeType frame_time = (post_sync - last_frame_time) - idle;
	last_frame_time = post_sync;
	STAT_SET(frame_time, frame_time);
	
#if ! defined(NDEBUG)
	// update fps
	memmove(_fps_history, _fps_history + 1, sizeof(* _fps_history) * (_fps_history_size - 1));
	_fps_history[_fps_history_size - 1] = sys::GetTime();
	STAT_SET (fps, float(_fps_history_size - 1) / float(_fps_history[_fps_history_size - 1] - _fps_history[0]));
#endif
	
	// Regulator feedback.
	TimeType target_frame_time = sim::Simulation::target_frame_seconds;
	target_frame_time *= target_work_proportion;
	
	formation_manager.SampleFrameRatio(frame_time, target_frame_time);
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
		Layer(Layer::background);
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
	
	Layer(Layer::foreground);
	
	RenderFormations();
	
	EndRenderForeground(pass);
	
	VerifyRenderState();
}

void Renderer::RenderFormations() const
{
	// Render the formations.
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	if (front_buffer.GetNumPolys() <= 0)
	{
		return;
	}
	
	form::FormationManager & fm = form::FormationManager::Daemon::Ref();
	fm.Render();
	
	// Draw the mesh!
	front_buffer.Bind();
	front_buffer.Activate(scene->GetPov());
	front_buffer.Draw();
	front_buffer.Deactivate();
	GLPP_VERIFY;
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

void Renderer::Layer(Layer::type layer) const
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
	STAT_SET (pos, pov.pos);	// std::streamsize previous_precision = out.precision(10); ...; out.precision(previous_precision);
	STAT_SET (rot, pov.rot);	// std::streamsize previous_precision = out.precision(2); ...; out.precision(previous_precision);
	
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

void Renderer::SetFence(gl::Fence & fence)
{
	if (fence.IsInitialized())
	{
		gl::SetFence(fence);
	}
}

void Renderer::FinishFence(gl::Fence & fence)
{
	if (fence.IsInitialized())
	{
		gl::FinishFence(fence);
	}
}


Renderer::Daemon * Renderer::singleton = nullptr;

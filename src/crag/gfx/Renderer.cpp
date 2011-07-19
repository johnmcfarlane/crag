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
#include "sim/Simulation.h"

#include <sstream>


#if ! defined(NDEBUG)
extern gl::TextureRgba8 const * test_texture;
#endif


namespace 
{

	//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
	CONFIG_DEFINE (background_ambient_color, gfx::Color4f, gfx::Color4f(0.1f));

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
, culling(init_culling)
, lighting(init_lighting)
, wireframe(init_wireframe)
, average_frame_time(0)
, frame_count(0)
, frame_count_reset_time(last_frame_time)
{
	sys::MakeCurrent();
	
	InitRenderState();

	Debug::Init();
}

gfx::Renderer::~Renderer()
{
	Debug::Deinit();

	init_culling = culling;
	init_lighting = lighting;
	init_wireframe = wireframe;
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
		Assert(param->enabled == gl::IsEnabled(param->cap));
		++ param;
	}
	
	// TODO: Write equivalent functions for all state in GL. :S
	Assert(gl::GetDepthFunc() == GL_LEQUAL);
	
	gfx::Color4f rgba;
	gl::GetColor(rgba.GetArray());
	Assert(rgba == gfx::Color4f::White());
#endif	// NDEBUG
}

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

sys::TimeType gfx::Renderer::Render(Scene & scene, bool enable_vsync)
{
	// Render the scene to the back buffer.
	RenderScene(scene);	

	sys::TimeType frame_time;
	
	// A guestimate of the time spent waiting for vertical synchronization. 
	sys::TimeType idle;
	
	// Flip the front and back buffers and get timing information.
	if (enable_vsync)
	{
		// I have no idea why this works, or seems to work
		// or works on my hardware, or even seems to work on my hardware
		// but it often does. I'm especially baffled by the glFlush().
		
		glFlush();
		sys::TimeType pre_finish = sys::GetTime();
		sys::SwapBuffers();
		glFinish();
		frame_time = sys::GetTime();
		idle = frame_time - pre_finish;
	}
	else
	{
		SDL_GL_SwapBuffers();
		frame_time = sys::GetTime();
		idle = 0;
	}
	
	STAT_SET(idle, idle);

	// Independant stat counters for measuring the FPS.
	++ frame_count;
	sys::TimeType fps_delta = frame_time - frame_count_reset_time;
	if (frame_count == 60) 
	{
		frame_count_reset_time = frame_time;
		average_frame_time = static_cast<float>(fps_delta) / frame_count;
		frame_count = 0;
	}
	
	// Calculate the amount of time from the last frame to this one.
	sys::TimeType frame_delta = frame_time - last_frame_time;
	Assert(frame_delta >= idle);
	last_frame_time = frame_time;

	// Return the frame delta - our basic performance metric.
	return frame_delta - idle * .5;
}

void gfx::Renderer::RenderScene(Scene const & scene) const
{
	VerifyRenderState();

	// The skybox, basically.
	RenderBackground(scene);
	
	// All the things.
	RenderForeground(scene);
	
	// Crap you generally don't want.
	DebugDraw(scene.pov);

	VerifyRenderState();
}

void gfx::Renderer::RenderBackground(Scene const & scene) const
{
	if (scene.skybox != nullptr) 
	{
		// Draw skybox.
		GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT));
		RenderSkybox(* scene.skybox, scene.pov);
	}
	else 
	{
		// Clear screen.
		GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	}
}

// Assumes projectin matrix is already set. Hoses modelview matrix.
void gfx::Renderer::RenderSkybox(Skybox const & skybox, Pov const & pov) const
{
	VerifyRenderState();
	
	// Set projection matrix within relatively tight bounds.
	Frustum skybox_frustum = pov.frustum;
	skybox_frustum.near_z = .1f;
	skybox_frustum.far_z = 10.f;
	skybox_frustum.SetProjectionMatrix();

	// Set matrix (minus the translation).
	sim::Matrix4 skybox_model_view_matrix = pov.CalcModelViewMatrix(false);
	gl::MatrixMode(GL_MODELVIEW);
	gl::LoadMatrix(skybox_model_view_matrix.GetArray());

	// Note: Skybox is being drawn very tiny but with z test off. This stops writing.
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	gl::Enable(GL_TEXTURE_2D);
	gl::Disable(GL_CULL_FACE);

	skybox.Draw();

	gl::Disable(GL_TEXTURE_2D);
	gl::Enable(GL_CULL_FACE);

	VerifyRenderState();
}

void gfx::Renderer::RenderForeground(Scene const & scene) const
{
	// Adjust near and far plane
	SetForegroundFrustum(scene, wireframe);
	
	if (wireframe)
	{
		RenderForegroundPass(scene, WireframePass1);
		RenderForegroundPass(scene, WireframePass2);
	}
	else 
	{
		RenderForegroundPass(scene, NormalPass);
	}
}

bool gfx::Renderer::BeginRenderForeground(Scene const & scene, ForegroundRenderPass pass) const
{
	Assert(gl::GetDepthFunc() == GL_LEQUAL);
	
	switch (pass) 
	{
		case NormalPass:
			if (! culling) {
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
		EnableLights(scene.lights, true);
	}
	
	gl::Enable(GL_DEPTH_TEST);
	
	return true;
}

// Each pass draws all the geometry. Typically, there is one per frame
// unless wireframe mode is on. 
void gfx::Renderer::RenderForegroundPass(Scene const & scene, ForegroundRenderPass pass) const
{
	if (! BeginRenderForeground(scene, pass))
	{
		return;
	}
	
	// Render the formations.
	form::FormationManager & fm = form::FormationManager::Ref();
	fm.Render(scene.pov);
	
	// Render everything else.
	for (Scene::EntityVector::const_iterator i = scene.entities.begin(), end = scene.entities.end(); i != end; ++ i)
	{
		sim::Entity const & entity = * * i;
		entity.Draw(scene);
	}
	
	EndRenderForeground(scene, pass);
}

void gfx::Renderer::EndRenderForeground(Scene const & scene, ForegroundRenderPass pass) const
{
	// Reset state
	if (lighting && pass != WireframePass2)
	{
		EnableLights(scene.lights, false);
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
}

// Nothing to do with shadow maps; sets/unsets the lights in the main scene.
void gfx::Renderer::EnableLights(std::vector<Light const *> const & lights, bool enabled) const
{
	if (enabled)
	{
		// TODO: Move this out of Render routine.
		GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
		GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));	// TODO: Broke!
		GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));
	}
	
	int light_id = GL_LIGHT0; 
	for (std::vector<Light const *>::const_iterator it = lights.begin(); it != lights.end(); ++ it) {
		Light const * light = * it;
		Assert(light != nullptr);
		
		if (light->IsActive()) {
			if (enabled)
			{
				gl::Enable(light_id);
				light->Draw(light_id);
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
	}
	
	while (light_id <= GL_LIGHT7) {
		Assert(! gl::IsEnabled(light_id));
		++ light_id;
	}
}

void gfx::Renderer::DebugDraw(Pov const & pov) const
{
#if defined(GFX_DEBUG)
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

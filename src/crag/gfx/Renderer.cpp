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
#include "Scene.h"
#include "Debug.h"
#include "Skybox.h"
#include "Light.h"

#include "form/Manager.h"

#include "sim/axes.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "core/Statistics.h"

#include "sim/Universe.h"

#include <sstream>


using sys::TimeType;
using sys::GetTime;


#if ! defined(NDEBUG)
extern gl::TextureRgba8 const * test_texture;
#endif


namespace 
{

	//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
	CONFIG_DEFINE (background_ambient_color, gfx::Color4f, gfx::Color4f(0.1f));

	CONFIG_DEFINE (init_culling, bool, true);
	CONFIG_DEFINE (init_lighting, bool, true);
	CONFIG_DEFINE (init_wireframe, bool, false);

	CONFIG_DEFINE (enable_shadow_mapping, bool, true);

	STAT (idle, double, .18);
	STAT (fps, float, .0);
	STAT (pos, sim::Vector3, .3);
	STAT (rot, sim::Matrix4, .9);
	
	void SetModelViewMatrix(sim::Matrix4 const & model_view_matrix) 
	{
		GLPP_CALL(glMatrixMode(GL_MODELVIEW));
		gl::LoadMatrix(model_view_matrix.GetArray());
	}

	void SetProjectionMatrix(sim::Matrix4 const & projection_matrix) 
	{
		GLPP_CALL(glMatrixMode(GL_PROJECTION));
		gl::LoadMatrix(projection_matrix.GetArray());
	}

	void SetFrustum(gfx::Frustum const & frustum)
	{
		gl::Viewport(0, 0, frustum.resolution.x, frustum.resolution.y);
		SetProjectionMatrix(frustum.CalcProjectionMatrix());
	}

	void SetForegroundFrustum(gfx::Pov const & pov, bool wireframe)
	{
		gfx::Frustum frustum = pov.frustum;
		frustum.near_z = std::numeric_limits<float>::max();

		sim::Ray3 camera_ray = axes::GetCameraRay(pov.pos, pov.rot);
		
		// TODO: This stuff belongs at the sim level - or at least outside the renderer.
		sim::Universe & universe = sim::Universe::Get();
		universe.GetRenderRange(camera_ray, frustum.near_z, frustum.far_z, wireframe);
		
		frustum.near_z = Max(frustum.near_z * .5, pov.frustum.near_z);
		SetFrustum(frustum);
	}
	
}	// namespace


gfx::Renderer::Renderer()
: last_frame_time(GetTime())
, culling(init_culling)
, lighting(init_lighting)
, wireframe(init_wireframe)
, average_frame_time(0)
, frame_count(0)
, frame_count_reset_time(last_frame_time)
{
	InitRenderState();
}

gfx::Renderer::~Renderer()
{
	init_culling = culling;
	init_lighting = lighting;
	init_wireframe = wireframe;
}

gfx::Renderer::StateParam const gfx::Renderer::init_state[] =
{
	{ GL_COLOR_MATERIAL, true },
	{ GL_TEXTURE_2D, false },
	{ GL_NORMALIZE, false },
	{ GL_CULL_FACE, true },
	{ GL_LIGHT0, false },
	{ GL_LIGHTING, false },
	{ GL_DEPTH_TEST, false },
	{ GL_BLEND, false },
	{ GL_INVALID_ENUM, false }
};

void gfx::Renderer::InitRenderState()
{
	StateParam const * param = init_state;
	while (param->cap != GL_INVALID_ENUM)
	{
		GLPP_CALL((param->enabled ? glEnable : glDisable)(param->cap));
		++ param;
	}

	GLPP_CALL(glFrontFace(GL_CCW));
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
		GLPP_CALL((param->enabled ? gl::CheckEnabled : gl::CheckDisabled)(param->cap));
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

TimeType gfx::Renderer::Render(Scene & scene, bool enable_vsync)
{
	// Render the scene to the back buffer.
	RenderScene(scene);	

	TimeType frame_time;
	
	// A guestimate of the time spent waiting for vertical synchronization. 
	TimeType idle;
	
	// Flip the front and back buffers and get timing information.
	if (enable_vsync)
	{
		// I have no idea why this works, or seems to work
		// or works on my hardware, or even seems to work on my hardware
		// but it often does. I'm especially baffled by the glFlush().
		
		glFlush();
		TimeType pre_finish = GetTime();
		SDL_GL_SwapBuffers();
		glFinish();
		frame_time = GetTime();
		idle = frame_time - pre_finish;
	}
	else
	{
		SDL_GL_SwapBuffers();
		frame_time = GetTime();
		idle = 0;
	}
	
	STAT_SET(idle, idle);

	// Independant stat counters for measuring the FPS.
	++ frame_count;
	sys::TimeType fps_delta = frame_time - frame_count_reset_time;
	if (frame_count == 60) 
	{
		frame_count_reset_time = frame_time;
		average_frame_time = static_cast<double>(fps_delta) / frame_count;
		frame_count = 0;
	}
	
	// Calculate the amount of time from the last frame to this one.
	sys::TimeType frame_delta = frame_time - last_frame_time;
	Assert(frame_delta >= idle);
	last_frame_time = frame_time;

	// Return the frame delta - our basic performance metric.
	return frame_delta - idle * .75;
}

void gfx::Renderer::RenderScene(Scene const & scene) const
{
	VerifyRenderState();

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

	// Adjust near and far plane
	SetForegroundFrustum(scene.pov, wireframe);
	
	if (wireframe)
	{
		RenderForeground(scene, WireframePass1);
		RenderForeground(scene, WireframePass2);
	}
	else 
	{
		RenderForeground(scene, NormalPass);
	}


	DebugDraw(scene.pov);

	VerifyRenderState();
}

// Assumes projectin matrix is already set. Hoses modelview matrix.
void gfx::Renderer::RenderSkybox(Skybox const & skybox, Pov const & pov) const
{
	VerifyRenderState();
	
	Frustum skybox_frustum = pov.frustum;
	skybox_frustum.near_z = .1f;
	skybox_frustum.far_z = 10.f;
	SetFrustum(skybox_frustum);

	SetModelViewMatrix(pov.CalcModelViewMatrix(false));

	// Note: Skybox is being drawn very tiny but with z test off. This stops writing.
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	gl::Enable(GL_TEXTURE_2D);
	gl::Disable(GL_CULL_FACE);

	skybox.Draw();

	gl::Disable(GL_TEXTURE_2D);
	gl::Enable(GL_CULL_FACE);

	VerifyRenderState();
}

void gfx::Renderer::RenderForeground(Scene const & scene, ForegroundRenderPass pass) const
{
	Assert(gl::GetDepthFunc() == GL_LEQUAL);
	
	bool color = true;
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
				return;
			}
			color = false;
			gl::SetColor<GLfloat>(0, 0, 0);
			gl::Enable(GL_POLYGON_OFFSET_FILL);
			GLPP_CALL(glPolygonOffset(1,1));
			break;
			
		case WireframePass2:
			if (! culling) {
				gl::Disable(GL_CULL_FACE);
			}
			//color = false;
			GLPP_CALL(glPolygonMode(GL_FRONT, GL_LINE));
			GLPP_CALL(glPolygonMode(GL_BACK, GL_LINE));
			break;
			
		default:
			Assert(false);
	}
	
	// Set state
	if (lighting)
	{
		gl::Enable(GL_LIGHTING);
	}
	
	gl::Enable(GL_DEPTH_TEST);

	// Do the rendering
	RenderLights(scene.lights);

	form::Manager & manager = form::Manager::Get();
	manager.Render(scene.pov, color);
	
	SetModelViewMatrix(scene.pov.CalcModelViewMatrix());
	
	gl::Disable(GL_LIGHT0);

	// Reset state
	if (lighting)
	{
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
			gl::SetColor<GLfloat>(1, 1, 1);
			gl::Disable(GL_POLYGON_OFFSET_FILL);
			break;
			
		case WireframePass2:
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

// Nothing to do with shadow maps; draws the lights in the main scene.
void gfx::Renderer::RenderLights(std::vector<Light const *> const & lights) const
{
	GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
	GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));	// TODO: Broke!
	GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));
	
	int light_id = GL_LIGHT0; 
	for (std::vector<Light const *>::const_iterator it = lights.begin(); it != lights.end(); ++ it) {
		Light const * light = * it;
		Assert(light != nullptr);
		
		if (light->IsActive()) {
			// TODO: Redundant state change?
			GLPP_CALL(glEnable(light_id));
			light->Draw(light_id);
			
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
		core::StatInterface const & stat = ref(* i);
		if (Debug::GetVerbosity() > stat.GetVerbosity())
		{
			out_stream << stat.GetName() << ": " << stat << '\n';
		}
	}
	
	Debug::DrawText(out_stream.str().c_str(), Vector2i::Zero());
#endif
	
#endif
}

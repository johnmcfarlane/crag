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
#include "Scene.h"
#include "Debug.h"
#include "Skybox.h"
#include "Light.h"

#include "form/Manager.h"

#include "sim/Entity.h"
#include "sim/Space.h"

#include "glpp/glpp.h"
#if (SHADOW_MAP_TEST >= 1)
#include "glpp/Vbo_Types.h"
#endif

#include "core/ConfigEntry.h"
#include "core/MatrixOps.h"
#include "core/Sphere3.h"


#if ! defined(NDEBUG)
extern gl::TextureRgba8 const * test_texture;
#endif


namespace ANONYMOUS {

//CONFIG_DEFINE (clear_color, Color, Color(1.f, 0.f, 1.f));
CONFIG_DEFINE (background_ambient_color, gfx::Color4f, gfx::Color4f(0.1f));
CONFIG_DEFINE (shadow_map_size, int, 128);

CONFIG_DEFINE (init_culling, bool, true);
CONFIG_DEFINE (init_lighting, bool, true);
CONFIG_DEFINE (init_shadow_mapping, bool, false);
CONFIG_DEFINE (init_smooth_shading, bool, true);
CONFIG_DEFINE (init_wireframe, bool, false);

CONFIG_DEFINE (enable_shadow_mapping, bool, true);


void SetModelViewMatrix(sim::Matrix4 const & model_view_matrix) 
{
	GLPP_CALL(glMatrixMode(GL_MODELVIEW));
	gl::LoadMatrix(model_view_matrix.GetArray());
}

void SetViewport(Vector2i const & resolution) 
{
	GLPP_CALL(glViewport(0, 0, resolution.x, resolution.y));
}

void SetProjectionMatrix(sim::Matrix4 const & projection_matrix) 
{
	GLPP_CALL(glMatrixMode(GL_PROJECTION));
	gl::LoadMatrix(projection_matrix.GetArray());
}

void SetFrustrum(class gfx::Frustrum const & frustrum)
{
	SetViewport(frustrum.resolution);
	SetProjectionMatrix(frustrum.CalcProjectionMatrix());
}

#if (SHADOW_MAP_TEST >= 1)

void InitShadowMapTest(gl::Vbo2dTex & quad_buffer)
{
	quad_buffer.Init();
	gl::Bind(& quad_buffer);
	quad_buffer.Resize(4);
	
	gl::Vertex2dTex quad[4];
	for (int i = 0; i < 4; ++ i) {
		quad[i].pos.x = (i & 1) ? .05f : .35f;
		quad[i].pos.y = (i & 2) ? .65f : .95f;
		quad[i].tex.x = (i & 1) ? 1.0f : 0.0f;
		quad[i].tex.y = (i & 2) ? 1.0f : 0.0f;
	}
	quad_buffer.Set(4, quad);
}

void DrawShadowMapTest(gl::Vbo2dTex & quad_buffer, std::map<gfx::ShadowMapKey, gfx::ShadowMap> const & shadow_maps)
{
	GLPP_VERIFY;

	for (gfx::ShadowMapMap::const_iterator it = shadow_maps.begin(); it != shadow_maps.end(); ++ it)
	{
		gfx::ShadowMap const & m = it->second;
		
#if (SHADOW_MAP_TEST >= 2)
		gl::TextureRgba8 const & t = m.GetColorTexture();
#else
		gl::TextureDepth const & t = m.GetDepthTexture();
#endif
		gl::Bind(& t);
		GLPP_VERIFY;
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		float w = 4.f / 3;
		glOrtho(0, w, 0, 1, 0, 1);
		
		//gl::Set<GL_DEPTH_FUNC>(GL_ALWAYS);
		gl::Disable(GL_CULL_FACE);
		
		gl::Disable(GL_LIGHTING);
		gl::Disable(GL_COLOR_MATERIAL);
		gl::Enable(GL_TEXTURE_2D);
		gl::Disable(GL_CULL_FACE);
		gl::Disable(GL_DEPTH_TEST);
		GLPP_VERIFY;
		
#if ! defined(NDEBUG)
		//Bind(test_texture);
		GLPP_VERIFY;
#endif
		
		gl::Bind(& quad_buffer);
		glColor3f(1,0,1);
		quad_buffer.Begin();
		quad_buffer.DrawStrip(0, 4);
		quad_buffer.End();
		GLPP_VERIFY;
		
		GLPP_VERIFY;
	}

	GLPP_VERIFY;
}

#else

void InitShadowMapTest(gl::Vbo2dTex &)
{
}

void DrawShadowMapTest(std::map<gfx::ShadowMapKey, gfx::ShadowMap> const &)
{
}

#endif

}


gfx::Renderer::Renderer()
: culling(init_culling)
, lighting(init_lighting)
, shadow_mapping(init_shadow_mapping)
, smooth_shading(init_smooth_shading)
, wireframe(init_wireframe)
{
	if (! HasShadowSupport() || true) {
		std::cout << "Renderer: Shadows not supported." << std::endl;
		shadow_mapping = false;
	}
	else {
		frame_buffer.Init();
		Bind(& frame_buffer);
		
		depth_buffer.Init();
		Bind(& depth_buffer);
		depth_buffer.ResizeForDepth(shadow_map_size, shadow_map_size);
		AttachToFrameBuffer(& depth_buffer);
		
		Bind((gl::FrameBuffer *)nullptr);
		
#if (SHADOW_MAP_TEST >= 1)
		InitShadowMapTest(quad_buffer);
#endif
	}

	InitRenderState();
}

gfx::Renderer::~Renderer()
{
	init_culling = culling;
	init_lighting = lighting;
	init_shadow_mapping = shadow_mapping;
	init_smooth_shading = smooth_shading;
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
	{ GL_INVALID_ENUM }
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
	gl::DepthFunc(GL_LEQUAL);
	GLPP_CALL(glPolygonMode(GL_FRONT, GL_FILL));
	GLPP_CALL(glPolygonMode(GL_BACK, GL_FILL));
	GLPP_CALL(glClearDepth(1.0f));
	GLPP_CALL(glColor4f(1, 1, 1, 1));

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
	
	Assert(gl::DepthFunc() == GL_LEQUAL);
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

void gfx::Renderer::ToggleShadowMapping()
{
	if (HasShadowSupport()) {
		shadow_mapping = ! shadow_mapping;
	}
}

void gfx::Renderer::ToggleSmoothShading()
{
	smooth_shading = ! smooth_shading;
}

void gfx::Renderer::ToggleWireframe()
{
	wireframe = ! wireframe;
}

void gfx::Renderer::Render(Scene & scene) const
{
	VerifyRenderState();

	if (shadow_mapping) {
		Assert (HasShadowSupport());
		GenerateShadowMaps(scene);
	}
	
	RenderScene(scene);

#if (SHADOW_MAP_TEST >= 1)
	if (shadow_mapping) {
		DrawShadowMapTest(quad_buffer, scene.shadow_maps);
	}
#endif

	VerifyRenderState();

	GLenum gl_error = glGetError(); 
	if (gl_error != GL_NO_ERROR) {
		std::cout << "gfx::Renderer::Render: " << gl_error << '\n';
		exit(1);
	}
}

void gfx::Renderer::RenderScene(Scene const & scene) const
{
	VerifyRenderState();

	if (scene.skybox != nullptr) {
		// Draw skybox.
		GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT));
		RenderSkybox(* scene.skybox, scene.pov);
	}
	else {
		// Clear screen.
		GLPP_CALL(glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
	}

	SetFrustrum(scene.pov.frustrum);
	
	if (wireframe)
	{
		RenderForeground(scene, WireframePass1);
		RenderForeground(scene, WireframePass2);
	}
	else {
		RenderForeground(scene, NormalPass);
	}


	DebugDraw(scene.pov);

	VerifyRenderState();
}

// Assumes projectin matrix is already set. Hoses modelview matrix.
void gfx::Renderer::RenderSkybox(Skybox const & skybox, Pov const & pov) const
{
	VerifyRenderState();
	
	Frustrum skybox_frustrum = pov.frustrum;
	skybox_frustrum.near_z = .1f;
	skybox_frustrum.far_z = 10.f;
	SetFrustrum(skybox_frustrum);

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
	Assert(gl::DepthFunc() == GL_LEQUAL);
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
			//GLPP_CALL(glCullFace(GL_FRONT));
			GLPP_CALL(glColor4f(0, 0, 0, 1));
			gl::Enable(GL_POLYGON_OFFSET_FILL);
			GLPP_CALL(glPolygonOffset(1,1));
			break;
			
		case WireframePass2:
			if (! culling) {
				gl::Disable(GL_CULL_FACE);
			}
			GLPP_CALL(glPolygonMode(GL_FRONT, GL_LINE));
			GLPP_CALL(glPolygonMode(GL_BACK, GL_LINE));
			break;
			
		default:
			Assert(false);
	}
	
	// Set state
	if (! smooth_shading) 
	{
		GLPP_CALL(glShadeModel(GL_FLAT));
	}
	if (lighting)
	{
		gl::Enable(GL_LIGHTING);
	}
	
	gl::Enable(GL_DEPTH_TEST);

	// Do the rendering
	if (! shadow_mapping) 
	{
		RenderSimpleLights(scene.lights);

		form::Manager & manager = form::Manager::Get();
		manager.Render(scene.pov, true);
	}
	
	SetModelViewMatrix(scene.pov.CalcModelViewMatrix());
	
	if (shadow_mapping) {
		RenderShadowLights(scene.pov, scene);
	}
	
	gl::Disable(GL_LIGHT0);

	// Reset state
	if (! smooth_shading) {
		GLPP_CALL(glShadeModel(GL_SMOOTH));
	}
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
			GLPP_CALL(glColor4f(1, 1, 1, 1));
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
void gfx::Renderer::RenderSimpleLights(std::vector<Light const *> const & lights) const
{
	GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
	GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));	// TODO: Broke!
	GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));
	
	int light_id = GL_LIGHT0; 
	for (std::vector<Light const *>::const_iterator it = lights.begin(); it != lights.end(); ++ it) {
		Light const * light = * it;
		Assert(light != nullptr);
		
		if (light->IsActive() && (! light->GenerateShadowMaps() || ! shadow_mapping)) {
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

void gfx::Renderer::RenderShadowLights(Pov const & pov, Scene const & scene) const
{
	if (! shadow_mapping) {
		return;
	}
	
	// Enable only LIGHT0
	GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gfx::Color4f(0.f)));	// TODO: Broke!
	GLPP_CALL(gl::Enable(GL_LIGHT0));
	for (int light_id = GL_LIGHT1; light_id <= GL_LIGHT7; ++ light_id) {
		// TODO: remove dupe disables
		GLPP_CALL(gl::Disable(light_id));
	}
	//RenderSimpleLights(scene.lights);
	
	// Set up other state stuff.
 	GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE));	// Separate view direction for each vert - rather than all parallel to z axis.
	GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gfx::Color4f(0.f)));
	//GLPP_CALL(glLightModelfv(GL_LIGHT_MODEL_AMBIENT, background_ambient_color));
	//GLPP_CALL(glLightModeli(GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR));

	gl::Enable(GL_TEXTURE_2D);
	
	//Set up texture coordinate generation.
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	
	gl::Enable(GL_TEXTURE_GEN_S);
	gl::Enable(GL_TEXTURE_GEN_T);
	gl::Enable(GL_TEXTURE_GEN_R);
	gl::Enable(GL_TEXTURE_GEN_Q);
	
    //Set alpha test to discard false comparisons
    glAlphaFunc(GL_GEQUAL, 0.99f);
    gl::Enable(GL_ALPHA_TEST);

	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	gl::Enable(GL_BLEND);
	
	// Z-sorting
//	gl::Set<GL_DEPTH_FUNC>(GL_LEQUAL);
//	GLPP_CALL(gl::Enable (GL_DEPTH_TEST));
	
	for (std::map<ShadowMapKey, ShadowMap>::const_iterator it = scene.shadow_maps.begin(); it != scene.shadow_maps.end(); ++ it)
	{
		ShadowMapKey const & key = it->first;
		
		Light const & light = * key.light;
		if (light.GenerateShadowMaps()) {
			light.Draw(GL_LIGHT0);
			
			ShadowMap const & map = it->second;
			RenderShadowLight(pov, key, map);
		}
	}
	
	//Disable textures and texgen
    gl::Disable(GL_TEXTURE_2D);
	
    gl::Disable(GL_TEXTURE_GEN_S);
    gl::Disable(GL_TEXTURE_GEN_T);
    gl::Disable(GL_TEXTURE_GEN_R);
    gl::Disable(GL_TEXTURE_GEN_Q);
	
    //Restore other states
    gl::Disable(GL_LIGHTING);
    gl::Disable(GL_ALPHA_TEST);
	gl::Disable(GL_BLEND);
	
	GLPP_VERIFY;
}

void gfx::Renderer::RenderShadowLight(Pov const & pov, ShadowMapKey const & key, ShadowMap const & map) const
{
	//Calculate texture matrix for projection
	//This matrix takes us from eye space to the light's clip space
	//It is postmultiplied by the inverse of the current view matrix when specifying texgen
	
	sim::Matrix4 biasMatrix(0.5f, 0.0f, 0.0f, 0.0f,
						0.0f, 0.5f, 0.0f, 0.0f,
						0.0f, 0.0f, 0.5f, 0.0f,
						0.5f, 0.5f, 0.5f, 1.0f);
	
	sim::Matrix4 light_proj = (map.light_proj);
	//Transpose(light_proj);
	
	sim::Matrix4 light_view = (map.light_view);
	//Transpose(light_view);

	// was biasMatrix * map.light_proj * map.light_view
	sim::Matrix4 textureMatrix = biasMatrix * light_proj * light_view;
	//sim::Matrix4 textureMatrix = (light_proj * light_view * biasMatrix);
	//sim::Matrix4 textureMatrix = (light_view * light_proj * biasMatrix);

	Transpose(textureMatrix);
	
	// TODO: GetColumn or GetRow??
	gl::TexGenv(GL_S, GL_EYE_PLANE, (textureMatrix)[0]);
	gl::TexGenv(GL_T, GL_EYE_PLANE, (textureMatrix)[1]);
	gl::TexGenv(GL_R, GL_EYE_PLANE, (textureMatrix)[2]);
	gl::TexGenv(GL_Q, GL_EYE_PLANE, (textureMatrix)[3]);
	
    //Bind & enable shadow map texture
	Bind(& map.GetDepthTexture());
	
#if (SHADOW_MAP_TEST >= 2)
	Bind(& map.GetColorTexture());
#endif
	
    //Enable shadow comparison
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
	
    //Shadow comparison should be true (ie not in shadow) if r<=texture
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
	
    //Shadow comparison should generate an INTENSITY result
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_ALPHA);
	
	// TODO: Only render key.entity
	std::vector<sim::Entity const *> entities;
	entities.push_back(key.entity);
	form::Manager::Get().Render(pov, true);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_NONE);
	GLPP_VERIFY;
}

void gfx::Renderer::DebugDraw(Pov const & pov) const
{
//	Pov test = pov;
//	test.frustrum.resolution = Vector2i(100, 100);
//	test.LookAtSphere(sim::Vector3(0, -200000.f, 0), sim::Sphere3(sim::Vector3::Zero(), 199999.f), Space::GetUp<float>());
//	Debug::AddFrustum(test);
	
	if (Debug::GetVerbosity() > .2) {
		Debug::out << "pos:" << pov.pos << '\n';
	}
	
	if (Debug::GetVerbosity() > .9) {
		std::streamsize previous_precision = Debug::out.precision(2);
		Debug::out << "rot:" << pov.rot << '\n';
		Debug::out.precision(previous_precision);
	}
	
#if defined(GFX_DEBUG)
	sim::Vector3 forward = Space::GetForward(pov.rot);
	sim::Vector3 ahead = pov.pos + (forward * (sim::Scalar)(pov.frustrum.near_z + 1));
	//Debug::DrawLine(ahead, ahead + sim::Vector3(1, 1, 1));
#endif
	
	Debug::DrawGraphics();
}

void gfx::Renderer::GenerateShadowMaps(Scene & scene) const
{
	// We only want to draw the backs of meshes for shadow maps.
	// This avoids a variation on z-fighting later down the line.
	GLPP_CALL(gl::Enable(GL_CULL_FACE));
	GLPP_CALL(glFrontFace(GL_CCW));
	GLPP_CALL(glCullFace(GL_FRONT));

	GLPP_CALL(gl::Disable(GL_BLEND));
	GLPP_CALL(glShadeModel(GL_FLAT));
	
	// Polygon Mode
	GLPP_CALL(glPolygonMode(GL_FRONT, GL_FILL));
	GLPP_CALL(glPolygonMode(GL_BACK, GL_FILL));
		
	Bind(& frame_buffer);
	
#if (SHADOW_MAP_TEST < 2)
	GLPP_CALL(glDrawBuffer(GL_NONE));
	GLPP_CALL(glReadBuffer(GL_NONE));
#endif

	ShadowMapKey key;	
	for (std::vector<Light const *>::const_iterator light_it = scene.lights.begin(); light_it != scene.lights.end(); ++ light_it) {
		key.light = * light_it;
		if (! key.light->IsActive() || ! key.light->GenerateShadowMaps()) {
			continue;
		}
		
		for (std::vector<sim::Entity const *>::const_iterator entity_it = scene.entities.begin(); entity_it != scene.entities.end(); ++ entity_it) {
			key.entity = * entity_it;
			if (! key.entity->IsShadowCatcher()) {
				continue;
			}
			
			GenerateShadowMap(scene, key);
		}
	}
	
	Bind((gl::FrameBuffer *)nullptr);	
}

//#include "Random.h"
void gfx::Renderer::GenerateShadowMap(Scene & scene, ShadowMapKey const & key) const
{
	// Set up light's point of view
	Pov light_pov;
	sim::Sphere3 sphere (key.entity->GetPosition(), key.entity->GetBoundingRadius());
	sim::Vector3 const & pos = key.light->GetPosition();
	light_pov.frustrum.resolution = Vector2i(shadow_map_size, shadow_map_size);
	light_pov.LookAtSphere(pos, sphere, Space::GetUp<double>());
#if 1
	//light_pov.pos = sim::Vector3(0, 200000, 0);
	//light_pov.frustrum.near_z = 100.f;
	//light_pov.frustrum.far_z = scene.pov.frustrum.far_z * 2;
	//light_pov.frustrum.fov = scene.pov.frustrum.fov;
#endif
	
	ShadowMap & s = scene.shadow_maps[key];
	s.Begin(shadow_map_size);
	s.light_proj = light_pov.frustrum.CalcProjectionMatrix();
	s.light_view = light_pov.CalcModelViewMatrix();
	
#if defined(GFX_DEBUG)
	Debug::AddFrustum(light_pov);
#endif
	
	// Render to shadow map texture.
	//GLPP_CALL(glClearDepth (Random::sequence.GetDoubleInclusive()));
	GLPP_CALL(glClearDepth(.5));

#if (SHADOW_MAP_TEST >= 2)
	GLPP_CALL(glClearColor(0, 1., 0, 1));
	// Note: Skybox doesn't work because frustum is too z-narrow.
	GLPP_CALL(glClear (GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT));
#else
	GLPP_CALL(glClear (GL_DEPTH_BUFFER_BIT));
#endif
	
	// viewport / projection
	SetViewport(light_pov.frustrum.resolution);
	SetProjectionMatrix(s.light_proj);
	
	SetModelViewMatrix(s.light_view);
	
	//RenderFormations(scene.entities, false);
	
	GLPP_VERIFY;
	
	if (Debug::GetVerbosity() > .9) {
		Debug::out << "light_pov.pos:" << light_pov.pos << '\n';
	}
	if (Debug::GetVerbosity() > .95) {
		std::streamsize previous_precision = Debug::out.precision(2);
		Debug::out << "light_pov.rot:" << light_pov.rot << '\n';
		Debug::out.precision(previous_precision);
	}
}

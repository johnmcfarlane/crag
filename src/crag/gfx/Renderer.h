/*
 *  Renderer.h
 *  Crag
 *
 *  Created by John on 12/19/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "ShadowMap.h"

#if (SHADOW_MAP_TEST >= 1)
#include "glpp/Vbo_Types.h"
#endif

#include "glpp/RenderBuffer.h"
#include "glpp/FrameBuffer.h"

#include "sim/defs.h"

#include <vector>


namespace sim
{
	class Entity;
}

namespace gfx
{
	class Light;
	class Pov;
	class ShadowMap;
	class ShadowMapKey;
	class Skybox;

	class Renderer
	{
	public:
		Renderer();
		~Renderer();
		
		void InitRenderState();
		void VerifyRenderState() const;

		bool HasShadowSupport() const;
		
		void ToggleCulling();
		void ToggleLighting();
		void ToggleShadowMapping();
		void ToggleSmoothShading();
		void ToggleWireframe();
		
		void Render(class Scene & scene) const;
	private:
		void RenderScene(Scene const & scene) const;
		void RenderSkybox(Skybox const & skybox, Pov const & pov) const;
		void RenderForeground(Scene const & scene) const;
		void RenderEntities(std::vector<sim::Entity const *> const & entities, Pov const & pov, bool color) const;
		void RenderSimpleLights(std::vector<Light const *> const & lights) const;
		
		void RenderShadowLights(Pov const & pov, Scene const & scene) const;
		void RenderShadowLight(Pov const & pov, ShadowMapKey const & key, ShadowMap const & map) const;
		
		void DebugDraw(Pov const & pov) const;
		
		void GenerateShadowMaps(Scene & scene) const;
		void GenerateShadowMap(Scene & scene, ShadowMapKey const & key) const;

		gl::FrameBuffer frame_buffer;
		gl::RenderBuffer depth_buffer;
		
		bool culling;
		bool lighting;
		bool shadow_mapping;
		bool smooth_shading;
		bool wireframe;

		struct StateParam
		{
			GLenum cap;
			bool enabled;
		};

		static StateParam const init_state[];

#if (SHADOW_MAP_TEST >= 1)
		mutable gl::Vbo2dTex quad_buffer;
#endif
	};
}

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
	class Scene;
	class Skybox;

	class Renderer
	{
		enum ForegroundRenderPass
		{
			NormalPass,
			WireframePass1,
			WireframePass2
		};
		
	public:
		Renderer();
		~Renderer();
		
		void InitRenderState();
		void VerifyRenderState() const;

		bool HasShadowSupport() const;
		
		void ToggleCulling();
		void ToggleLighting();
		void ToggleWireframe();
		
		void Render(Scene & scene) const;
	private:
		void RenderScene(Scene const & scene) const;
		void RenderSkybox(Skybox const & skybox, Pov const & pov) const;
		void RenderForeground(Scene const & scene, ForegroundRenderPass pass) const;
		//void RenderEntities(std::vector<sim::Entity const *> const & entities, Pov const & pov, bool color) const;
		void RenderLights(std::vector<Light const *> const & lights) const;
		
		void DebugDraw(Pov const & pov) const;
		
		gl::FrameBuffer frame_buffer;
		gl::RenderBuffer depth_buffer;
		
		bool culling;
		bool lighting;
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

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

#include "sys/App.h"


namespace sim
{
	class Entity;
}


namespace gfx
{
	// forward-declarations
	class Light;
	class Pov;
	class Scene;
	class Skybox;

	// The Renderer class. 
	// Does all the donkey-work of bullying OpenGL 
	// into turning the simulated world
	// into an array of pixels.
	class Renderer
	{
		OBJECT_NO_COPY(Renderer);
		
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
		
		// Returns the estimated time thread was busy this frame.
		sys::TimeType Render(Scene & scene, bool enable_vsync);
	private:
		void RenderScene(Scene const & scene) const;
		void RenderSkybox(Skybox const & skybox, Pov const & pov) const;
		void RenderForeground(Scene const & scene, ForegroundRenderPass pass) const;
		//void RenderEntities(std::vector<sim::Entity const *> const & entities, Pov const & pov, bool color) const;
		void EnableLights(std::vector<Light const *> const & lights, bool enabled) const;
		
		void DebugDraw(Pov const & pov) const;

		////////////////////////////////////////////////////////////////////////////////
		// Attributes
		
		gl::FrameBuffer frame_buffer;
		gl::RenderBuffer depth_buffer;
		
		sys::TimeType last_frame_time;
		
		bool culling;
		bool lighting;
		bool wireframe;

		struct StateParam
		{
			GLenum cap;
			bool enabled;
#if ! defined(NDEBUG)
			char const * name;
#endif
		};

		static StateParam const init_state[];

		float average_frame_time;
		int frame_count;
		sys::TimeType frame_count_reset_time;
	};
}

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

#include "defs.h"

#include "glpp/RenderBuffer.h"
#include "glpp/FrameBuffer.h"

#include "smp/Actor.h"

#include "sys/App.h"

#include "geom/Matrix4.h"


namespace sim
{
	class Entity;
	struct SetCameraMessage;
}


namespace gfx
{
	// forward-declarations
	class Light;
	class Object;
	class Pov;
	class Scene;
	class Skybox;
	
	
	struct AddObjectMessage;
	struct RemoveObjectMessage;
	struct RenderMessage { };
	struct RendererReadyMessage { };

	
	struct ResizeMessage
	{
		Vector2i size;
	};
	
	// The Renderer class. 
	// Does all the donkey-work of bullying OpenGL 
	// into turning the simulated world
	// into an array of pixels.
	class Renderer : public smp::Actor<Renderer>
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
		
		// Singleton
		static Renderer & Ref() { return ref(singleton); }
		
		// Message passing
		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message) 
		{ 
			Renderer & destination = Ref(); 
			smp::Actor<Renderer>::SendMessage(destination, message); 
		}

		void OnMessage(smp::TerminateMessage const & message);
		void OnMessage(AddObjectMessage const & message);
		void OnMessage(RemoveObjectMessage const & message);
		template <typename OBJECT> void OnMessage(UpdateObjectMessage<OBJECT> const & message)
		{
			message._object = message._updated;
		}
		void OnMessage(RenderMessage const & message);
		void OnMessage(ResizeMessage const & message);
		void OnMessage(sim::SetCameraMessage const & message);

	private:
		virtual void Run();
		void MainLoop();
		
		void InitRenderState();
		void VerifyRenderState() const;

		bool HasShadowSupport() const;
		
	public:
		void ToggleCulling();
		void ToggleLighting();
		void ToggleWireframe();
		
	private:
		// Returns the estimated time thread was busy this frame.
		void Render();
		void RenderScene() const;
		
		void RenderBackground() const;
		
		void RenderForeground() const;
		bool BeginRenderForeground(ForegroundRenderPass pass) const;
		void RenderForegroundPass(ForegroundRenderPass pass) const;
		void EndRenderForeground(ForegroundRenderPass pass) const;
		
		void EnableLights(bool enabled) const;
		
		// returns the number of objects renderer
		void Draw(RenderStage::type render_stage) const;
		void Draw(ObjectVector const & objects) const;
		
		void DebugDraw() const;

		////////////////////////////////////////////////////////////////////////////////
		// Attributes
		
		Scene * scene;

		//gl::FrameBuffer frame_buffer;
		//gl::RenderBuffer depth_buffer;
		
		sys::TimeType last_frame_time;
		
		bool quit_flag;
		bool culling;
		bool lighting;
		bool wireframe;
		int render_flag;

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
		
		static Renderer * singleton;
	};
}

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

#include "glpp/Fence.h"
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
	class Scene;
	
	struct AddObjectMessage;
	struct RemoveObjectMessage;
	struct RenderReadyMessage { bool ready; };
	
	
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
		
		typedef smp::Actor<Renderer> super;
		
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
		void OnMessage(RenderReadyMessage const & message);
		void OnMessage(ResizeMessage const & message);
		void OnMessage(sim::SetCameraMessage const & message);

	private:
		virtual void Run();
		void MainLoop();
		void ProcessMessagesAndGetReady();

		void Init();
		void Deinit();
		
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
		void RenderStage(RenderStage::type render_stage) const;
		void RenderStage(ObjectVector const & objects) const;
		
		void DebugDraw() const;

		////////////////////////////////////////////////////////////////////////////////
		// Attributes
		
		Scene * scene;

		//gl::FrameBuffer frame_buffer;
		//gl::RenderBuffer depth_buffer;
		
		sys::TimeType last_frame_time;
		
		bool quit_flag;
		bool ready;
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
		
#if ! defined(NDEBUG)
		// fps counter
		static int const _fps_history_size = 60;
		sys::TimeType _fps_history[_fps_history_size];
#endif
		
		gl::Fence _fence1, _fence2;
		
		static Renderer * singleton;
	};
}

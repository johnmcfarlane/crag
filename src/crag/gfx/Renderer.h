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

#include "Image.h"

#include "glpp/Fence.h"
#include "glpp/FrameBuffer.h"
#include "glpp/RenderBuffer.h"

#include "smp/Daemon.h"

#include "sys/App.h"

#include "geom/Matrix44.h"

#include "core/double_buffer.h"


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
	struct ToggleCullingMessage { };
	struct ToggleLightingMessage { };
	struct ToggleWireframeMessage { };
	struct ToggleCaptureMessage { };
	
	
	struct ResizeMessage
	{
		Vector2i size;
	};
	
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
		typedef smp::Daemon<Renderer> Daemon;
		
		Renderer();
		~Renderer();
		
		void OnMessage(smp::TerminateMessage const & message);
		void OnMessage(AddObjectMessage const & message);
		void OnMessage(RemoveObjectMessage const & message);
		template <typename OBJECT> void OnMessage(UpdateObjectMessage<OBJECT> const & message)
		{
			message._object.Update(message._params);
		}
		void OnMessage(RenderReadyMessage const & message);
		void OnMessage(ResizeMessage const & message);
		void OnMessage(ToggleCullingMessage const & message);
		void OnMessage(ToggleLightingMessage const & message);
		void OnMessage(ToggleWireframeMessage const & message);
		void OnMessage(ToggleCaptureMessage const & message);
		void OnMessage(sim::SetCameraMessage const & message);

		void Run(Daemon::MessageQueue & message_queue);
	private:
		void MainLoop();
		void ProcessMessagesAndGetReady(Daemon::MessageQueue & message_queue);

		bool Init();
		void Deinit();
		
		void InitRenderState();
		void VerifyRenderState() const;

		bool HasShadowSupport() const;
		
		void PreRender();		
		void Render();
		void RenderScene() const;
		
		void RenderBackground() const;
		
		void RenderForeground() const;
		bool BeginRenderForeground(ForegroundRenderPass pass) const;
		void RenderForegroundPass(ForegroundRenderPass pass) const;
		void EndRenderForeground(ForegroundRenderPass pass) const;
		
		void EnableLights(bool enabled) const;
		
		void RenderLayer(Layer::type layer) const;
		
		void DebugDraw() const;
		void ProcessRenderTiming();
		void Capture();

		static void SetFence(gl::Fence & fence);

		////////////////////////////////////////////////////////////////////////////////
		// Attributes
		
		Scene * scene;

		//gl::FrameBuffer frame_buffer;
		//gl::RenderBuffer depth_buffer;
		
		sys::TimeType last_frame_time;
		
		bool quit_flag;
		bool ready;
		bool vsync;
		bool culling;
		bool lighting;
		bool wireframe;
		int capture_frame;
		
		gfx::Image capture_image[2];

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
		
		static Daemon * singleton;
	};
}

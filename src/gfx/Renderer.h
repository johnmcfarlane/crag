//
//  Renderer.h
//  crag
//
//  Created by John on 12/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"
#include "Layer.h"
#include "Image.h"

#include "glpp/Fence.h"
#include "glpp/FrameBuffer.h"
#include "glpp/RenderBuffer.h"

#include "smp/Daemon.h"

#include "core/double_buffer.h"


namespace sim
{
	class Entity;
	struct SetCameraMessage;
}


namespace gfx
{
	// forward-declarations
	class BranchNode;
	class Scene;
	

	// gfx::Daemon type
	class Renderer;
	typedef smp::Daemon<Renderer> Daemon;
	
	
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
		
		Scene const & GetScene() const;

		// message interface
		void OnQuit();
		void OnAddObject(Object * const & object, Uid const & parent_uid);
		void OnRemoveObject(Uid const & uid);
		void OnSetReady(bool const & ready, Time const & time);
		void OnResize(Vector2i const & size);
		void OnToggleCulling();
		void OnToggleLighting();
		void OnToggleWireframe();
		void OnToggleCapture();
		void OnSetCamera(Transformation const & transformation);

		// defined in Renderer.inl
		template <typename OBJECT> 
		void OnUpdateObject(Uid const & uid, typename OBJECT::UpdateParams const & params);

		void Run(Daemon::MessageQueue & message_queue);
	private:
		void MainLoop();
		void ProcessMessagesAndGetReady(Daemon::MessageQueue & message_queue);

		bool Init();
		void Deinit();
		
		void InitVSync();
		void InitRenderState();
		void VerifyRenderState() const;

		bool HasShadowSupport() const;
		
		void PreRender();
		void UpdateTransformations(BranchNode & node, gfx::Transformation const & model_view_transformation);
		void UpdateTransformations();
		void Render();
		void RenderScene() const;
		
		void RenderBackground() const;
		
		void RenderForeground() const;
		bool BeginRenderForeground(ForegroundRenderPass pass) const;
		void RenderForegroundPass(ForegroundRenderPass pass) const;
		void EndRenderForeground(ForegroundRenderPass pass) const;
		
		int RenderLayer(Layer::type layer, bool opaque = true) const;
		
		void DebugDraw() const;
		void ProcessRenderTiming();
		void Capture();

		static void SetFence(gl::Fence & fence);

		////////////////////////////////////////////////////////////////////////////////
		// Attributes
		
		SDL_GLContext context;
		Scene * scene;

		//gl::FrameBuffer frame_buffer;
		//gl::RenderBuffer depth_buffer;
		
		Time last_frame_time;
		
		bool quit_flag;
		bool _ready;
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
		Time _fps_history[_fps_history_size];
#endif
		
		gl::Fence _fence1, _fence2;
		
		//static Daemon * singleton;
	};
}

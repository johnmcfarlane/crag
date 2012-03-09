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
#include "Fence.h"
#include "FrameBuffer.h"
#include "Layer.h"
#include "Image.h"

#include "smp/Daemon.h"

#include "core/double_buffer.h"


namespace sim
{
	struct SetCameraMessage;
}


namespace gfx
{
	// forward-declarations
	class BranchNode;
	class MeshResource;
	class Program;
	class ResourceManager;
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

		Object * GetObject(Uid const & uid);
		Object const * GetObject(Uid const & uid) const;
		
		Scene & GetScene();
		Scene const & GetScene() const;
		
		ResourceManager & GetResourceManager();
		ResourceManager const & GetResourceManager() const;
		
		Program const * GetCurrentProgram() const;
		void SetCurrentProgram(Program const * program);
		
		MeshResource const * GetCurrentMesh() const;
		void SetCurrentMesh(MeshResource const * mesh);
		
		Color4f CalculateLighting(Vector3 const & position) const;

		// message interface
		void OnQuit();
		void OnAddObject(Object & object);
		void OnRemoveObject(Uid const & uid);
		void OnSetParent(Uid const & child_uid, Uid const & parent_uid);
		void OnSetParent(Object & child, Uid const & parent_uid);
		void OnSetParent(Object & child, BranchNode & parent);
		void OnSetTime(Time const & time);
		void OnSetReady(bool const & ready);
		void OnResize(Vector2i const & size);
		void OnToggleCulling();
		void OnToggleLighting();
		void OnToggleWireframe();
		void OnToggleCapture();
		void OnSetCamera(Transformation const & transformation);

		template <typename OBJECT_TYPE, typename INIT_DATA>
		void OnCreateObject(Uid const & uid, INIT_DATA const & init_data)
		{
			OBJECT_TYPE * object = new OBJECT_TYPE;
			object->SetUid(uid);
			object->Init(* this, init_data);
			OnAddObject(* object);
		}
		
		void Run(Daemon::MessageQueue & message_queue);
	private:
		void MainLoop();
		void ProcessMessagesAndGetReady(Daemon::MessageQueue & message_queue);
		bool ProcessMessage(Daemon::MessageQueue & message_queue);

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
		void RenderScene();
		
		void RenderBackground();
		
		void RenderForeground();
		void RenderLights();
		bool BeginRenderForeground(ForegroundRenderPass pass) const;
		void RenderForegroundPass(ForegroundRenderPass pass);
		void EndRenderForeground(ForegroundRenderPass pass) const;
		
		int RenderLayer(Layer::type layer, bool opaque = true);
		
		void DebugDraw();

		void ProcessRenderTiming();
		void GetRenderTiming(Time & frame_start_position, Time & pre_sync_position, Time & post_sync_position);
		void ConvertRenderTiming(Time frame_start_position, Time pre_sync_position, Time post_sync_position, Time & frame_duration, Time & busy_duration);
		void UpdateRegulator(Time busy_duration) const;
		void UpdateFpsCounter(Time frame_start_position);
		
		void Capture();

		static void SetFence(Fence & fence);

		////////////////////////////////////////////////////////////////////////////////
		// Attributes
		
		SDL_GLContext context;
		Scene * scene;
		ResourceManager * _resource_manager;

		//gl::FrameBuffer frame_buffer;
		//gl::RenderBuffer depth_buffer;
		
		Time last_frame_end_position;
		
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
		static int const _frame_time_history_size = 360;
		Time _frame_time_history[_frame_time_history_size];
#endif
		
		Fence _fence1, _fence2;

		Program const * _current_program;
		MeshResource const * _current_mesh;
	};
}

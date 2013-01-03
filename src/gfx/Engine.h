//
//  Engine.h
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
#include "RenderBuffer.h"
#include "Texture.h"

#include "smp/Daemon.h"
#include "smp/EngineBase.h"

#include "core/double_buffer.h"
#include "core/Statistics.h"

DECLARE_CLASS_HANDLE(form, RegulatorScript);


namespace gfx
{
	// forward-declarations
	class BranchNode;
	class MeshResource;
	class Program;
	class ResourceManager;
	class Scene;

	// gfx::Daemon type
	class Engine;
	typedef smp::Daemon<Engine> Daemon;
	
	
	// The graphics Engine class. 
	// Does all the donkey-work of bullying OpenGL 
	// into turning the simulated world
	// into an array of pixels.
	class Engine : public smp::EngineBase<Engine, Object>
	{
		OBJECT_NO_COPY(Engine);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		enum ForegroundRenderPass
		{
			NormalPass,
			WireframePass1,
			WireframePass2
		};
		
	public:
		typedef smp::EngineBase<Engine, Object> super;
		typedef smp::Daemon<Engine> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();

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
		virtual void OnAddObject(Object & object) override final;
		virtual void OnRemoveObject(Object & object) override final;
		void OnSetParent(Uid child_uid, Uid parent_uid);
		void OnSetParent(Object & child, Uid parent_uid);
		void OnSetParent(Object & child, BranchNode & parent);
		void OnSetTime(core::Time time);
		void OnSetReady(bool ready);
		void OnResize(geom::Vector2i size);
		void OnToggleCulling();
		void OnToggleLighting();
		void OnToggleWireframe();
		void OnToggleFlatShaded();
		void OnToggleFragmentLighting();
		void OnToggleCapture();
		void OnSetCamera(Transformation const & transformation);
		Transformation const& GetCamera() const;
		void OnSetRegulatorHandle(form::RegulatorScriptHandle regulator_handle);

		void Run(Daemon::MessageQueue & message_queue);
	private:
		void MainLoop();
		bool ProcessMessage(Daemon::MessageQueue & message_queue);

		bool Init();
		void Deinit();
		bool InitFrameBuffer();
		
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
#if defined(VERIFY)
		virtual void Verify() const override;
#endif

		void ProcessRenderTiming();
		void GetRenderTiming(core::Time & frame_start_position, core::Time & pre_sync_position, core::Time & post_sync_position);
		void ConvertRenderTiming(core::Time frame_start_position, core::Time pre_sync_position, core::Time post_sync_position, core::Time & frame_duration, core::Time & busy_duration);
		void UpdateRegulator(core::Time busy_duration) const;
#if defined(GATHER_STATS)
		void UpdateFpsCounter(core::Time frame_start_position);
#endif
		
		void Capture();

		static void SetFence(Fence & fence);

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		SDL_GLContext context;
		Scene * scene;
		ResourceManager * _resource_manager;

		//FrameBuffer frame_buffer;
		//RenderBuffer depth_buffer;
		//Texture depth_texture;
		
		core::Time _frame_duration;
		core::Time last_frame_end_position;
		
		bool quit_flag;
		bool _ready;
		bool _dirty;
		bool vsync;
		bool culling;
		bool lighting;
		bool wireframe;
		bool _flat_shaded;
		bool _fragment_lighting;
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
		core::Time _frame_time_history[_frame_time_history_size];
#endif
		
		Fence _fence1, _fence2;

		Program const * _current_program;
		MeshResource const * _current_mesh;
		smp::Handle<form::RegulatorScript> _regulator_handle;
	};
}

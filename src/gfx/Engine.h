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
#include "FrameBuffer.h"
#include "Image.h"
#include "LightType.h"
#include "RenderBuffer.h"
#include "Texture.h"

#if defined(WIN32)
#include "SetCameraEvent.h"
#include "SetSpaceEvent.h"
#endif

#include "ipc/Daemon.h"
#include "ipc/EngineBase.h"
#include "ipc/Listener.h"

#include "geom/Space.h"

#include "core/ResourceHandle.h"
#include "core/Statistics.h"

namespace gfx
{
	// forward-declarations
	class Light;
	class Scene;
	struct SetCameraEvent;
	struct SetSpaceEvent;

	// gfx::Daemon type
	class Engine;
	typedef ipc::Daemon<Engine> Daemon;

	DECLARE_CLASS_HANDLE(Object);	// gfx::ObjectHandle
	
	// The graphics Engine class. 
	// Does all the donkey-work of bullying OpenGL 
	// into turning the simulated world
	// into an array of pixels.
	class Engine 
	: public ipc::EngineBase<Engine, Object>
	, private ipc::Listener<Engine, SetCameraEvent>
	, private ipc::Listener<Engine, SetSpaceEvent>
	{
		OBJECT_NO_COPY(Engine);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef ipc::Listener<gfx::Engine, gfx::SetCameraEvent> SetCameraListener;
		typedef ipc::Listener<gfx::Engine, gfx::SetSpaceEvent> SetSpaceListener;
	public:
		typedef ipc::EngineBase<Engine, Object> super;
		typedef ipc::Daemon<Engine> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Engine);

		Engine();
		~Engine();

		ResourceManager & GetResourceManager();
		ResourceManager const & GetResourceManager() const;

		Scene & GetScene();
		Scene const & GetScene() const;
		
		ProgramHandle GetCurrentProgram() const;
		void SetCurrentProgram(ProgramHandle program);
		
		VboResourceHandle GetVboResource() const;
		void SetVboResource(VboResourceHandle vbo);
		
		// message interface
		void OnQuit();
		void OnAddObject(ObjectSharedPtr const & object) final;
		void OnRemoveObject(ObjectSharedPtr const & object) final;
		void OnSetParent(ObjectHandle child_uid, ObjectHandle parent_uid);
		void OnSetParent(Object & child, ObjectHandle parent_uid);
		void OnSetParent(Object & child, Object & parent);
		void OnSetTime(core::Time time);
		void OnSetReady(bool ready);
		void OnToggleCulling();
		
		void OnToggleCapture();
		void operator() (const SetCameraEvent & event) final;

		void operator() (const SetSpaceEvent & event) final;
		geom::Space const & GetSpace() const;
		
		void SetIsSuspended(bool suspended);
		bool GetIsSuspended() const;

		void Run(Daemon::MessageQueue & message_queue);
	private:
		void MainLoop();
		bool ProcessMessage(Daemon::MessageQueue & message_queue);

		void Init();
		void Deinit();
		
		void InitVSync();
		void InitRenderState();
		void VerifyRenderState() const;

		void PreRender();
		void UpdateTransformations(Object & node, Transformation const & model_view_transformation);
		void UpdateTransformations();
		void UpdateShadowVolumes();
		void Render();
		void RenderFrame();
		void RenderScene();
		
		void RenderBackgroundPass(Matrix44 const & projection_matrix);
		void RenderTransparentPass(Matrix44 const & projection_matrix);
		
		void RenderLayer(Matrix44 const & projection_matrix, Layer layer, LightFilter const & light_filter, bool add_ambient);
		void RenderShadowLights(Matrix44 const & projection_matrix);
		void RenderShadowLight(Matrix44 const & projection_matrix, Light & light);
		void RenderShadowVolumes(Matrix44 const & projection_matrix, Light & light);
		
#if defined(CRAG_GFX_DEBUG)
		void DebugDraw(Matrix44 const & projection_matrix);
#endif
#if defined(GATHER_STATS)
		void DebugText();
#endif

		void ProcessRenderTiming();
		void GetRenderTiming(core::Time & frame_start_position, core::Time & frame_end_position);
		void SampleFrameDuration(core::Time busy_duration) const;
#if defined(GATHER_STATS)
		void UpdateFpsCounter(core::Time frame_start_position);
#endif
		
		void Capture();

		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		ResourceManager & _resource_manager;

		Scene & _scene;
		geom::Space _space;
		
		core::Time _target_frame_duration;
		core::Time last_frame_end_position;
		
		bool quit_flag;
		bool _ready;
		bool _dirty;
		bool culling;
		bool _suspended;
		
		int capture_frame;

		Image capture_image[2];

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
		static int constexpr _frame_time_history_size = 60;
		std::array<core::Time, _frame_time_history_size> _frame_time_history;
#endif
		
		ProgramHandle _current_program;
		VboResourceHandle _current_vbo;
	};
}

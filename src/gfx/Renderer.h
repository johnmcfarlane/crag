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
#include "Program.h"

#include "glpp/Fence.h"
#include "glpp/FrameBuffer.h"
#include "glpp/Shader.h"

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
	class Cuboid;
	class Scene;
	class Quad;

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
		
		Program * GetProgram(ProgramIndex::type index);
		Program const * GetProgram(ProgramIndex::type index) const;
		
		void SetCurrentProgram(Program * program);
		Program const * GetCurrentProgram() const;
		
		Cuboid const & GetCuboid() const;
		Quad const & GetSphereQuad() const;
		Quad const & GetDiskQuad() const;

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
		bool InitShaders();
		bool InitGeometry();
		void Deinit();
		
		bool InitObject(Object & object);
		
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

		static void SetFence(gl::Fence & fence);

		////////////////////////////////////////////////////////////////////////////////
		// Attributes
		
		SDL_GLContext context;
		Scene * scene;

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
		static int const _frame_time_history_size = 60;
		Time _frame_time_history[_frame_time_history_size];
#endif
		
		gl::Fence _fence1, _fence2;
		
		// shaders and shader programs
		gl::Shader _light_frag_shader;
		Program * _programs[ProgramIndex::max_index];
		Program * _current_program;

		// stock geometry
		Cuboid * _cuboid;
		Quad * _sphere_quad;
		Quad * _disk_quad;
	};
}

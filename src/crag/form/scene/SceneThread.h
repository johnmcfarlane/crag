/*
 *  form/SceneThread.h
 *  Crag
 *
 *  Created by John on 10/12/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Regulator.h"
#include "Scene.h"

#include "smp/Semaphore.h"
#include "smp/Thread.h"

#include "glpp/glpp.h"

#include "core/double_buffer.h"


namespace form
{
	
	// The scene is the workhorse of the formation system. 
	// SceneThread provides a thread-safe interface to the rest of the system.
	// Optionally, it can also run the scene without multithreading.
	class SceneThread
	{
		OBJECT_NO_COPY (SceneThread);
		
	public:
		SceneThread(FormationSet const & _formations, bool _threaded);
		~SceneThread();
		
#if VERIFY
		void Verify() const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(SceneThread);
		
		void Launch();
		void ToggleSuspended();
		void Quit();
		
		void SampleFrameRatio(float ratio);
		void ResetRegulator();

		void Tick();
		void ForEachFormation(FormationFunctor & f) const;
		bool PollMesh(MeshBufferObject & mbo);
		void SetCameraPos(sim::Ray3 const & camera_pos);
		void ResetOrigin();
		void ToggleFlatShaded();
		
		bool IsOriginOk() const;
	private:
		bool IsResetting() const;
		bool IsGrowing() const;

		Scene & GetActiveScene();
		Scene const & GetActiveScene() const;

		Scene & GetVisibleScene();
		Scene const & GetVisibleScene() const;
		
		void Run();	// Called when thread is launched and runs the loop.
		void TickThread();
		void TickActiveScene();

		void BeginReset();
		void EndReset();

		void AdjustNumQuaterna();

		void GenerateMesh();
		
		bool IsMainThread() const;
		bool IsSceneThread() const;
		
		// The front scene is always the one being displayed and is usually the 'active' scene.
		// During an origin reset, the back scene is the active one.
		// It's the active scene which is reshaped for LODding purposes (churned).
		core::double_buffer<Scene> scenes;
		bool is_in_reset_mode;	// the scene is being regenerated following an origin reset
				
		FormationSet const & formations;		
		sim::Ray3 camera_pos;
		
		bool threaded;	// If false, object still does the same work, only on the main thread.
		bool reset_origin_flag;	// When set, tells thread to generate a new scene with a new origin.
		bool suspend_flag;
		bool quit_flag;
		
		// The mesh is the client-side arrays of verts and indices
		// that are generated by the scenes
		// and get passed to the VBOs.
		core::double_buffer<Mesh *> meshes;
		volatile bool back_buffer_ready;

		sys::TimeType mesh_generation_time;

		typedef smp::Thread<SceneThread, & SceneThread::Run> Thread;
		Thread thread;
		
		smp::Semaphore suspend_semaphore;
		Regulator regulator;
	};
}


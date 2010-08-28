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

#include "Mesh.h"
#include "Scene.h"

#include "sys/Mutex.h"
#include "sys/Thread.h"

#include "glpp/glpp.h"

#include "sys/App.h"


namespace sim
{
	class Observer;
}


namespace form
{
	
	// The scene is the workhorse of the formation system. 
	// SceneThread provides a thread-safe interface to the rest of the system.
	// Optionally, it can also run the scene without multithreading.
	class SceneThread
	{
	public:
		SceneThread(FormationSet const & _formations, sim::Observer const & _observer, bool _threaded);
		
#if VERIFY
		void Verify() const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(SceneThread);
		
		void Launch();
		void Quit();
		
		//int GetTargetNumNodes() const;
		void SetFrameRatio(float ratio);

		int GetNumQuaternaUsed() const;
		int GetNumQuaternaUsedTarget() const;
		
		void Tick();
		void ForEachFormation(FormationFunctor & f) const;
		bool PollMesh(form::MeshBufferObject & mbo);
		void ResetOrigin();
		bool PostResetFreeze() const;
		void ToggleFlatShaded();
		
		bool IsOriginOk() const;
		
	private:
		void Run();	// Called when thread is launched and runs the loop.
		void ThreadTick();
		
		void AdjustNumQuaterna();
		static int CalculateFrameRateDirectedTargetNumQuaterna(int current_num_quaterna, float frame_ratio);
		static int CalculateMeshGenerationDirectedTargetNumQuaterna(int current_num_quaterna, float mesh_generation_period);
		
		bool GenerateMesh();
		
		bool IsMainThread() const;
		bool IsSceneThread() const;

		Scene scene;
		int num_nodes;
		float frame_ratio;	// ~ (actual framerate / ideal framerate)
		sys::Mutex scene_mutex;
		double scene_tick_period;
		
		FormationSet const & formations;		
		sim::Observer const & observer;
		
		bool threaded;	// If false, object still does the same work, only on the main thread.
		bool reset_origin_flag;	// When set, a mesh is generated (flushed) and the origin/scene is reset.
		bool suspend_flag;
		bool quit_flag;
		
		app::TimeType origin_reset_time;

		// The mesh is the client-side arrays of verts and indices
		// that are generated by the scenes
		// and get passed to the VBOs.
		form::Mesh mesh;
		sim::Vector3 mesh_origin;
		sys::Mutex mesh_mutex;
		bool mesh_updated;
		app::TimeType mesh_generation_time;
		double mesh_generation_period;

		typedef sys::Thread<SceneThread, & form::SceneThread::Run> Thread;
		Thread * thread;
	};
}


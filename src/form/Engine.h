//
//  Engine.h
//  crag
//
//  Created by john on 5/23/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/scene/Mesh.h"
#include "form/scene/Scene.h"

#include "sim/defs.h"

#include "core/Singleton.h"

#include "ipc/Daemon.h"
#include "ipc/EngineBase.h"
#include "ipc/Listener.h"
#include "smp/scheduler.h"
#include "smp/Semaphore.h"


namespace gfx 
{
	DECLARE_CLASS_HANDLE(FormationMesh);	// gfx::FormationMeshHandle
	struct SetCameraEvent;
	struct SetOriginEvent;
}

namespace form 
{	
	////////////////////////////////////////////////////////////////////////////////
	// forward declarations
	
	class FormationFunctor;
	class Formation;
	class Scene;
	
	typedef std::set<Formation *> FormationSet;
	
	
	// form::Daemon type
	class Engine;
	typedef ipc::Daemon<Engine> Daemon;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// form::Engine class
	
	// The top-most formation management class.
	class Engine 
	: public ipc::EngineBase<Engine, Formation>
	, private ipc::Listener<Engine, gfx::SetCameraEvent>
	, private ipc::Listener<Engine, gfx::SetOriginEvent>
	{
		OBJECT_SINGLETON(Engine);
		
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef ipc::Listener<Engine, gfx::SetCameraEvent> SetCameraListener;
		typedef ipc::Listener<Engine, gfx::SetOriginEvent> SetOriginListener;

		enum {
			// tweakables
			min_num_quaterne = 1024,
			max_num_quaterne = 65536
		};
		
	public:
		typedef ipc::Daemon<Engine> Daemon;

		struct TreeQueryFunctor
		{
			virtual ~TreeQueryFunctor() { }
			virtual void operator () (Scene const & scene) = 0;
			
		};
		
		typedef core::ring_buffer<smp::scheduler::Job> BatchedFunctorBuffer;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();
		
#if defined(VERIFY)
		// Verification
		void Verify() const;
#endif
		
		// message interface
		void OnQuit();
		void OnAddFormation(Formation & formation);
		void OnRemoveFormation(Formation & formation);
		void OnSetMesh(Mesh & mesh);
		void operator() (gfx::SetCameraEvent const & event) final;
		void operator() (gfx::SetOriginEvent const & event) final;
		
		void EnableAdjustNumQuaterna(bool enabled);
		void OnSetRecommendedNumQuaterne(int recommented_num_quaterne);
		
		void OnToggleSuspended();
		void OnToggleMeshGeneration();
		
		void Run(Daemon::MessageQueue & message_queue);

	private:
		void Tick();
		void TickScene();
		void GenerateMesh();
		Mesh * PopMesh();
		
		void AdjustNumQuaterna();
		
		void OnOriginReset();
		bool IsGrowing() const;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		gfx::FormationMeshHandle _mesh;
		FormationSet _formations;
		
		bool quit_flag;
		bool suspend_flag;
		bool enable_mesh_generation;
		
		Mesh::list_type _meshes;
		
		core::Time mesh_generation_time;
		
		bool _enable_adjust_num_quaterna;
		int _requested_num_quaterne;
		
		bool _pending_origin_request;

		geom::rel::Ray3 _camera;
		geom::abs::Vector3 _origin;
		Scene _scene;
	};
	
}

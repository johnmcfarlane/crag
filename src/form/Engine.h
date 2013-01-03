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

#include "smp/Daemon.h"
#include "smp/EngineBase.h"
#include "smp/scheduler.h"
#include "smp/Semaphore.h"


DECLARE_CLASS_HANDLE(gfx, FormationMesh)	// gfx::FormationMeshHandle
DECLARE_CLASS_HANDLE(form, RegulatorScript)	// form::RegulatorScriptHandle


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
	typedef smp::Daemon<Engine> Daemon;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// form::Engine class
	
	// The top-most formation management class.
	class Engine : public smp::EngineBase<Engine, Formation>
	{
		OBJECT_SINGLETON(Engine);
		
		enum {
			// tweakables
			min_num_quaterne = 1024,
			max_num_quaterne = 65536
		};
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef smp::Daemon<Engine> Daemon;

		struct TreeQueryFunctor
		{
			virtual ~TreeQueryFunctor() { }
			virtual void operator () (Scene const & scene) = 0;
			
		};
		
		typedef core::ring_buffer<smp::scheduler::Job, true> BatchedFunctorBuffer;
		
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
		void SetCamera(geom::rel::Ray3 const & camera_position);
		void OnSetOrigin(geom::abs::Vector3 const & origin);
		
		void OnRegulatorSetEnabled(bool enabled);
		void OnSetRecommendedNumQuaterne(int recommented_num_quaterne);
		
		void OnToggleSuspended();
		void OnToggleMeshGeneration();
		
		void Run(Daemon::MessageQueue & message_queue);

	private:
		void Tick();
		void TickScene();
		void GenerateMesh();
		void BroadcastFormationUpdates();
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
		
		bool _regulator_enabled;
		RegulatorScriptHandle _regulator_handle;
		int _recommended_num_quaterne;	// recommended by the regulator
		
		bool _pending_origin_request;

		geom::rel::Ray3 _camera;
		Scene _scene;
	};
	
}

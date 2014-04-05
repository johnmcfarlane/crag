//
//  Engine.h
//  crag
//
//  Created by john on 5/23/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/Scene.h"

#include "gfx/LodParameters.h"

#include "core/Singleton.h"

#include "ipc/Daemon.h"
#include "ipc/EngineBase.h"
#include "ipc/Listener.h"

namespace gfx 
{
	DECLARE_CLASS_HANDLE(Surrounding);	// gfx::SurroundingHandle
	struct SetLodParametersEvent;
	struct SetOriginEvent;
}

namespace form 
{	
	////////////////////////////////////////////////////////////////////////////////
	// forward declarations
	
	class Formation;
	class Mesh;
	
	typedef std::set<Formation *> FormationSet;
	
	// form::Daemon type
	class Engine;
	typedef ipc::Daemon<Engine> Daemon;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// form::Engine class
	
	// The top-most formation management class.
	class Engine 
	: public ipc::EngineBase<Engine, Formation>
	, private ipc::Listener<Engine, gfx::SetOriginEvent>
	, private ipc::Listener<Engine, gfx::SetLodParametersEvent>
	{
		OBJECT_SINGLETON(Engine);
		
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef ipc::EngineBase<Engine, Formation> super;
		typedef ipc::Listener<Engine, gfx::SetOriginEvent> SetOriginListener;
		typedef ipc::Listener<Engine, gfx::SetLodParametersEvent> SetLodParametersListener;

	public:
		typedef ipc::Daemon<Engine> Daemon;

		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Engine);
		
		// message interface
		void OnQuit();
		void OnAddFormation(Formation & formation);
		void OnRemoveFormation(Formation & formation);
		void OnSetMesh(std::shared_ptr<Mesh> const & mesh);
		void operator() (gfx::SetLodParametersEvent const & event) final;
		void operator() (gfx::SetOriginEvent const & event) final;
		
		void EnableAdjustNumQuaterna(bool enabled);
		void OnSetRecommendedNumQuaterne(std::size_t recommented_num_quaterne);
		
		void OnToggleSuspended();
		void OnToggleMeshGeneration();

		void Run(Daemon::MessageQueue & message_queue);

	private:
		void Tick();
		void TickScene();
		void GenerateMesh();
		std::shared_ptr<Mesh> PopMesh();
		
		void AdjustNumQuaterna();
		
		void OnOriginReset();
		bool IsGrowing() const;
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		gfx::SurroundingHandle _mesh;
		FormationSet _formations;
		
		bool quit_flag;
		bool suspend_flag;
		bool enable_mesh_generation;
		
		std::stack<std::shared_ptr<Mesh>> _meshes;
		
		core::Time mesh_generation_time;
		
		bool _enable_adjust_num_quaterna;
		std::size_t _requested_num_quaterne;
		
		bool _pending_origin_request;

		gfx::LodParameters _lod_parameters;
		geom::abs::Vector3 _origin;
		Scene _scene;
	};
	
}

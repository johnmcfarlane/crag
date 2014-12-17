//
//  Engine.h
//  crag
//
//  Created by John on 10/19/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "ipc/Daemon.h"
#include "ipc/EngineBase.h"
#include "ipc/Listener.h"

#if defined(WIN32_C2079_WORKAROUND)
#include "gfx/SetCameraEvent.h"
#include "gfx/SetSpaceEvent.h"
#endif
#include "gfx/SetLodParametersEvent.h"

#include "geom/Space.h"

#include "core/Singleton.h"

namespace core
{
	namespace locality
	{
		class Roster;
	}
}

namespace physics 
{
	class Engine;
}

namespace form
{
	class Formation;
	class Scene;
}

namespace gfx
{
	struct SetCameraEvent;
	struct SetLodParametersEvent;
	struct SetSpaceEvent;
}

namespace sim
{
	
	// forward declarations
	class Entity;
	
	// Engine - main object of simulation thread
	class Engine 
	: public ipc::EngineBase<Engine, Entity>
	, private ipc::Listener<Engine, gfx::SetCameraEvent>
	, private ipc::Listener<Engine, gfx::SetSpaceEvent>
	, private ipc::Listener<Engine, gfx::SetLodParametersEvent>
	{
		OBJECT_SINGLETON(Engine);

	public:
		typedef ipc::EngineBase<Engine, Entity> super;
		typedef ipc::Daemon<Engine> Daemon;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Engine();
		~Engine();
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Engine);
		
		// message interface
		void OnQuit();
		
		void OnAddObject(ObjectSharedPtr const & entity) final;
		
		void AddFormation(form::Formation& formation);
		void RemoveFormation(form::Formation& formation);
		
		void operator() (gfx::SetCameraEvent const & event) final;
		Ray3 const & GetCamera() const;

		void operator() (gfx::SetSpaceEvent const & event) final;
		geom::Space const & GetSpace() const;
		
		void operator() (gfx::SetLodParametersEvent const & event) final;
		gfx::LodParameters const & GetLodParameters() const;
		
		void IncrementPause(int increment);
		bool IsPaused() const;
		
		void OnToggleGravity();
		void OnToggleCollision();

		// accessors
		core::Time GetTime() const;
		physics::Engine & GetPhysicsEngine();		

		form::Scene & GetScene();
		form::Scene const & GetScene() const;
		
		bool IsSettled() const;
		
		core::locality::Roster & GetTickRoster();
		core::locality::Roster & GetDrawRoster();

		// called be Daemon when simulation thread starts
		void Run(Daemon::MessageQueue & message_queue);
	private:
		void Tick();
		void UpdateRenderer() const;

		// call Tick on entities in the given object
		void PurgeEntities();
		
		////////////////////////////////////////////////////////////////////////////////
		// attributes
		
		bool quit_flag;
		int _pause_counter = 0;
		
		core::Time _time;

		Ray3 _camera;
		geom::Space _space;
		gfx::LodParameters _lod_parameters;
		physics::Engine & _physics_engine;
		form::Scene & _collision_scene;	// for collision

		core::locality::Roster & _tick_roster;	// general simulation tick
		core::locality::Roster & _draw_roster;	// provides opportunity to communicate graphical changes to renderer
	};
	
}

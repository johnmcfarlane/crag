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

#if defined(WIN32)
#include "gfx/SetCameraEvent.h"
#include "gfx/SetOriginEvent.h"
#endif
#include "gfx/SetLodParametersEvent.h"

#include "geom/origin.h"

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
	struct SetOriginEvent;
}

namespace sim
{
	
	// forward declarations
	class Entity;
	
	// Engine - main object of simulation thread
	class Engine 
	: public ipc::EngineBase<Engine, Entity>
	, private ipc::Listener<Engine, gfx::SetCameraEvent>
	, private ipc::Listener<Engine, gfx::SetOriginEvent>
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
		
		virtual void OnAddObject(super::Object & entity) override final;
		void OnAttachEntities(Uid uid1, Uid uid2);
		
		void AddFormation(form::Formation& formation);
		void RemoveFormation(form::Formation& formation);
		
		void operator() (gfx::SetCameraEvent const & event) final;
		Ray3 const & GetCamera() const;

		void operator() (gfx::SetOriginEvent const & event) final;
		geom::abs::Vector3 const & GetOrigin() const;
		
		void operator() (gfx::SetLodParametersEvent const & event) final;
		gfx::LodParameters const & GetLodParameters() const;
		
		void OnTogglePause();
		void OnToggleGravity();
		void OnToggleCollision();

		// accessors
		core::Time GetTime() const;
		physics::Engine & GetPhysicsEngine();		

		form::Scene & GetScene();
		form::Scene const & GetScene() const;
		
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
		bool paused;
		
		core::Time _time;

		Ray3 _camera;
		geom::abs::Vector3 _origin;
		gfx::LodParameters _lod_parameters;
		physics::Engine & _physics_engine;
		form::Scene & _collision_scene;	// for collision

		core::locality::Roster & _tick_roster;	// general simulation tick
		core::locality::Roster & _draw_roster;	// provides opportunity to communicate graphical changes to renderer
	};
	
}

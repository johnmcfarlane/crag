//
//  Entity.h
//  crag
//
//  Created by john on 5/21/09.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ipc/ObjectBase.h"

#include "defs.h"

#include "core/RosterObjectDeclare.h"

namespace gfx
{
	DECLARE_CLASS_HANDLE(Object);	// gfx::ObjectHandle
}

namespace physics
{
	class Location;
}

namespace sim
{
	class Controller;

	// A thing that exist in the simulation.
	class Entity : public ipc::ObjectBase<Entity, sim::Engine>
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
	public:
		using super = ipc::ObjectBase<Entity, sim::Engine>;

		using ControllerPtr = std::unique_ptr<Controller>;
		using LocationPtr = std::unique_ptr<physics::Location>;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		Entity(Engine & engine);
		~Entity();

		CRAG_ROSTER_OBJECT_DECLARE(Entity);

		// general callbacks
		void Tick();

		// controller
		void SetController(ControllerPtr controller);
		Controller * GetController();
		Controller const * GetController() const;
		
		// physics
		void SetLocation(LocationPtr locator);
		physics::Location * GetLocation();
		physics::Location const * GetLocation() const;

		// graphics
		gfx::ObjectHandle GetModel() const;
		void SetModel(gfx::ObjectHandle model);
		void UpdateModels();
		
		// Verification
		CRAG_VERIFY_INVARIANTS_DECLARE(Entity);

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		ControllerPtr _controller;
		LocationPtr _location;
		gfx::ObjectHandle _model;
	};
}

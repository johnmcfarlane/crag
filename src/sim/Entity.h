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

namespace physics
{
	class Location;
}

namespace sim
{
	class Controller;
	class Model;

	// A thing that exist in the simulation.
	class Entity : public ipc::ObjectBase<Entity, sim::Engine>
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
	public:
		using super = ipc::ObjectBase<Entity, sim::Engine>;

		using ControllerPtr = std::unique_ptr<Controller>;
		using LocationPtr = std::unique_ptr<physics::Location>;
		using ModelPtr = std::unique_ptr<Model>;

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
		void SetModel(ModelPtr model);
		Model * GetModel();
		Model const * GetModel() const;

		// Verification
		CRAG_VERIFY_INVARIANTS_DECLARE(Entity);

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		ControllerPtr _controller;
		LocationPtr _location;
		ModelPtr _model;
	};
}

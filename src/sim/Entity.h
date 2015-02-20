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

		using ControllerPtr = std::shared_ptr<Controller>;
		using LocationPtr = std::shared_ptr<physics::Location>;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(Entity);

		Entity(Engine & engine);
		~Entity();
		
		// general callbacks
		static void Tick(Entity *);

		// controller
		void SetController(ControllerPtr const & controller);
		ControllerPtr & GetController();
		ControllerPtr const & GetController() const;
		
		// physics
		void SetLocation(LocationPtr const & locator);
		LocationPtr & GetLocation();
		LocationPtr const & GetLocation() const;

		// graphics
		gfx::ObjectHandle GetModel() const;
		void SetModel(gfx::ObjectHandle model);
		static void UpdateModels(Entity const *);
		
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

//
//  Entity.h
//  crag
//
//  Created by john on 5/21/09.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace gfx
{
	DECLARE_CLASS_HANDLE(BranchNode);	// gfx::BranchNodeHandle
}

namespace physics
{
	class Body;
	class Location;
}

namespace sim
{
	class Controller;

	// A thing that exist in the simulation.
	class Entity : public smp::Object<Entity, sim::Engine>
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef smp::Object<Entity, sim::Engine> super;
	public:

		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(Vehicle);

		Entity(Init const & init);
		~Entity();
		
		// general callbacks
		void Tick();

		// controller
		void SetController(Controller * controller);
		Controller * GetController();
		
		// physics
		void SetLocation(physics::Location * locator);
		physics::Location * GetLocation();
		physics::Location const * GetLocation() const;
		physics::Body * GetBody();
		physics::Body const * GetBody() const;

		// graphics
		gfx::BranchNodeHandle GetModel() const;
		void SetModel(gfx::BranchNodeHandle model);
		void UpdateModels() const;

		// Verification
	#if defined(VERIFY)
		virtual void Verify() const final;
	#endif

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		Controller * _controller;
		physics::Location * _location;
		gfx::BranchNodeHandle _model;
	};
}

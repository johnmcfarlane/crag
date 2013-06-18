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
	DECLARE_CLASS_HANDLE(Object);	// gfx::ObjectHandle
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
	class Entity : public ipc::Object<Entity, sim::Engine>
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef ipc::Object<Entity, sim::Engine> super;
	public:

		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(Entity);

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
		gfx::ObjectHandle GetModel() const;
		void SetModel(gfx::ObjectHandle model);
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
		gfx::ObjectHandle _model;
	};
}

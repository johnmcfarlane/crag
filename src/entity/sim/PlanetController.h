//
//  PlanetController.h
//  crag
//
//  Created by john on 4/13/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/defs.h"
#include "sim/Controller.h"

namespace gfx
{
	DECLARE_CLASS_HANDLE(Planet);	// gfx::PlanetHandle
}

namespace form
{
	class Formation;
	class Object;
	DECLARE_CLASS_HANDLE(Object);	// form::ObjectHandle
}

namespace sim
{
	// A planet is a celestial body which is represented using the formation system.
	class PlanetController : public Controller
	{
		// types
		using FormationPtr = std::unique_ptr<form::Formation>;
		
	public:
		// functions
		CRAG_ROSTER_OBJECT_DECLARE(PlanetController);

		PlanetController(Entity & entity, Sphere3 const & sphere, int random_seed, int num_craters);
		~PlanetController();

		form::Formation const & GetFormation() const;

	private:
		void Tick();
		
		// variables
		FormationPtr _formation;	// physics
		form::ObjectHandle _handle;	// graphics
		Scalar _radius_mean;
	};
}

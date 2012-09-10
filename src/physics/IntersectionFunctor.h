//
//  IntersectionFunctor.h
//  crag
//
//  Created by John McFarlane on 9/13/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "sim/defs.h"


namespace physics
{
	// forward-declarations
	class Engine;
	
	// the functor which is called for each intersection between a formation and a physics::Body.
	class IntersectionFunctor
	{
	public:
		IntersectionFunctor(Engine & physics_engine, dGeomID object_geom, dGeomID planet_geom);
		
		Engine const & GetEngine() const;
		
		void operator()(sim::Vector3 const & pos, sim::Vector3 const & normal, sim::Scalar depth);		
		
	private:
		// variables
		Engine & _engine;
		dContact _contact;
	};
}

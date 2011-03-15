/*
 *  PlanetCollisionFunctor.h
 *  crag
 *
 *  Created by John on 6/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "form/FormationFunctor.h"

#include "physics/defs.h"


namespace sim 
{
	
	////////////////////////////////////////////////////////////////////////////////
	// PlanetCollisionFunctor class

	// The base class for formation functors which test for collision between a planet and an ODE geom.
	// Currently, the only child class is PlaneSphereCollisionFunctor. 
	
	class PlanetCollisionFunctor : public form::FormationFunctor 
	{
	public:
		PlanetCollisionFunctor(form::Formation const & in_planet_formation, dGeomID in_planet_geom, dGeomID in_object_geom);
		
	protected:
		void OnContact(Vector3 const & pos, Vector3 const & normal, Scalar depth);
		
		form::Formation const & planet_formation;
		dContact contact;
	};
	
}

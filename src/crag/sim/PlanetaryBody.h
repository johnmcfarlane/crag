/*
 *  FormationBody.h
 *  crag
 *
 *  Created by John on 6/20/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "physics/SphericalBody.h"


namespace form
{
	class Formation;
}


namespace sim
{
	// Planets are roughly sphere shaped, 
	// so a spherical body is used to detect near collision,
	// then PlanetaryBody does the work of colliding with the planet's surface.
	
	class PlanetaryBody : public physics::SphericalBody
	{
	public:
		PlanetaryBody(physics::Engine & physics_engine, form::Formation const & formation, physics::Scalar radius);
		
		form::Formation const & GetFormation() const;
	private:
		virtual bool OnCollision(physics::Engine & engine, Body & that_body);
		virtual bool OnCollisionWithSphericalBody(physics::Engine & engine, SphericalBody & that_sphere);
		
		form::Formation const & formation;
	};
	
}

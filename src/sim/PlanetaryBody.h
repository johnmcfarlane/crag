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
		
	private:
		virtual bool OnCollision(physics::Engine & engine, Body const & that_body) const;

		virtual void OnDeferredCollisionWithBox(physics::Body const & body, physics::IntersectionFunctor & functor) const;
		virtual void OnDeferredCollisionWithSphere(physics::Body const & body, physics::IntersectionFunctor & functor) const;

		// variables
		form::Formation const & _formation;
	};
	
}
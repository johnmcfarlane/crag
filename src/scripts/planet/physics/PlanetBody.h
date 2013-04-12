//
//  PlanetaryBody.h
//  crag
//
//  Created by John on 6/20/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "physics/defs.h"

#include "physics/SphericalBody.h"

namespace form
{
	class Formation;
}

namespace physics
{
	// Planets are roughly sphere shaped, 
	// so a spherical body is used to detect near collision,
	// then PlanetaryBody does the work of colliding with the planet's surface.
	
	class PlanetBody : public SphericalBody
	{
	public:
		DECLARE_ALLOCATOR(PlanetaryBody);

		PlanetBody(Engine & physics_engine, form::Formation const & formation, Scalar radius);
		
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
	private:
		virtual bool OnCollision(Engine & engine, Body const & that_body) const;

		virtual void OnDeferredCollisionWithBox(Body const & body, IntersectionFunctor & functor) const;
		virtual void OnDeferredCollisionWithRay(Body const & body, IntersectionFunctor & functor) const;
		virtual void OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctor & functor) const;

		// variables
		form::Formation const & _formation;
		Scalar _mean_radius;
	};
	
}

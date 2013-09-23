//
//  PlanetBody.h
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
	class Scene;
}

namespace physics
{
	// Planets are roughly sphere shaped, 
	// so a spherical body is used to detect near collision,
	// then PlanetBody does the work of colliding with the planet's surface.
	
	class PlanetBody : public SphericalBody
	{
		////////////////////////////////////////////////////////////////////////////////
		// functions

	public:
		DECLARE_ALLOCATOR(PlanetBody);

		PlanetBody(Transformation const & transformation, Engine & engine, form::Formation const & formation, Scalar radius);
		
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
	private:
		virtual bool OnCollision(Body & body);

		bool OnCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere) final;
		bool OnCollisionWithRay(Body & body) final;

		// variables
		form::Formation const & _formation;
		Scalar _mean_radius;
	};
	
}
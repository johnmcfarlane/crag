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
	class Polyhedron;
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

		PlanetBody(Transformation const & transformation, Engine & engine, form::Polyhedron const & polyhedron, Scalar radius);
		
		Vector3 GetGravitationalForce(Vector3 const & pos) const final;
	private:
		virtual bool OnCollision(Body & body, ContactInterface & contact_interface);

		bool OnCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere, ContactInterface & contact_interface) final;
		bool OnCollisionWithRay(Body & body) final;

		// variables
		form::Polyhedron const & _polyhedron;
		Scalar _mean_radius;
	};
	
}

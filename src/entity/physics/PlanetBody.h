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

#include "physics/SphereBody.h"

namespace form
{
	class Polyhedron;
}

namespace physics
{
	// Planets are roughly sphere shaped, 
	// so a spherical body is used to detect near collision,
	// then PlanetBody does the work of colliding with the planet's surface.
	
	class PlanetBody final : public SphereBody
	{
		////////////////////////////////////////////////////////////////////////////////
		// functions

	public:
		CRAG_ROSTER_OBJECT_DECLARE(PlanetBody);

		PlanetBody(Transformation const & transformation, Engine & engine, form::Polyhedron const & polyhedron, Scalar radius);
		
		Vector3 GetGravitationalAttraction(Vector3 const & pos) const override;
	private:
		bool HandleCollision(Body & body, ContactFunction & contact_function) override;

		bool HandleCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere, ContactFunction & contact_function) override;
		bool HandleCollisionWithRay(Body & body) override;

		void DebugDraw() const override;

		// variables
		form::Polyhedron const & _polyhedron;
		Scalar _mean_radius;
	};
	
}

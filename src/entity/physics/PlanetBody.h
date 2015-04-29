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
		PlanetBody(Transformation const & transformation, Engine & engine, form::Polyhedron const & polyhedron, Scalar radius);
		
		Vector3 GetGravitationalAttraction(Vector3 const & pos) const override;
	private:
		virtual bool OnCollision(Body & body, ContactInterface & contact_interface);

		bool OnCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere, ContactInterface & contact_interface) override;
		bool OnCollisionWithRay(Body & body) override;

		void DebugDraw() const override;

		// variables
		form::Polyhedron const & _polyhedron;
		Scalar _mean_radius;
	};
	
}

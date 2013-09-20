//
//  form/CastRay.h
//  crag
//
//  Created by John McFarlane on 2013-08-29
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "defs.h"

namespace form 
{
	////////////////////////////////////////////////////////////////////////////////
	// forward-declaration

	class Polyhedron;

	////////////////////////////////////////////////////////////////////////////////
	// RayCastResult class definition - represents contact between ray and polyhedron

	struct RayCastResult
	{
		// this type dictates the precision used in CastRay
		typedef ::form::Scalar Scalar;

		// the normal of the surface with which the ray contacted
		Vector3 normal;

		// the distance along the ray (as proportion of ray direction)
		// at which contact occurred
		Scalar projection = std::numeric_limits<Scalar>::max();

		// the individual node with which contact occurred
		Node const * node = nullptr;

#if defined(VERIFY)
		void Verify() const;
#endif
	};

	// performs a ray cast on a polyhedron; ray must have unit direction 
	// and length is the minimum guaranteed distance along ray to scan
	RayCastResult CastRay(Polyhedron const & polyhedron, Ray3 const & ray, Scalar length);
}

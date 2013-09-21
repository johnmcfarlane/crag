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
	// forward-declarations

	class Polyhedron;
	class RayCastResult;

	// performs a ray cast on a polyhedron; ray must have unit direction 
	// and if no contact is found, length is max for Scalar
	RayCastResult CastRay(Polyhedron const & polyhedron, Ray3 const & ray, Scalar length);
}

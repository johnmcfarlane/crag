//
//  Distance.h
//  crag
//
//  Created by John McFarlane on 9/14/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
//  Functions for determining the distance between geometric entities.
//

#pragma once

#include "Plane.h"
#include "Triangle.h"

namespace geom
{
	////////////////////////////////////////////////////////////////////////////////
	// Points

	template<typename S, int N> S Distance(Vector<S, N> const & a, Vector<S, N> const & b)
	{
		return Length(a - b);
	}

	template<typename S, int N> S DistanceSq(Vector<S, N> const & a, Vector<S, N> const & b)
	{
		return LengthSq(a - b);
	}


	////////////////////////////////////////////////////////////////////////////////
	// Surfaces

	// Distance from given surface, to point, p.
	// Result is signed. If surface.direction isn't unit, result will be proportional.
	template<typename S, int N> S Distance(Plane<S, N> const & surface, Vector<S, N> const & p) 
	{
		return DotProduct(surface.normal, p - surface.position);
	}

	// Distance from plane represented by triangle, t, to point, p.
	// Result is signed. 
	template<typename S, int N> S Distance(Triangle<S, N> const & triangle, Vector<S, N> const & point) 
	{
		Plane<S, N> plane(triangle);
		Normalize(plane.normal);
		
		return Distance(plane, point);
	}

	// Distance from plane represented by triangle, t, to point, p.
	// Result is signed. Uses faster, less precise operations.
	template<typename S, int N> S FastDistance(Triangle<S, N> const & triangle, Vector<S, N> const & point) 
	{
		Plane<S, N> plane;
		plane.position = triangle.points[1];
		plane.normal = FastNormalized(Normal(triangle));
		
		return Distance(plane, point);
	}
}

/*
 *  SphereUtils.h
 *  Crag
 *
 *  Created by John on 1/5/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "floatOps.h"
#include "NSphere.h"


// Point-Circle Intersection Test

template<typename V, typename S> bool Contains(Sphere<V, S> const & sphere, V const & point)
{
	S center_distance_squared = LengthSq(sphere.center - point);
	return center_distance_squared <= Square(sphere.radius);
}

template<typename V, typename S> bool IsInside(V const & point, Sphere<V, S> const & sphere)
{
	return Contains(sphere, point);
}


// Circle-Circle Intersection Test

template<typename V, typename S> bool Touches(Sphere<V, S> const & a, Sphere<V, S> const & b)
{
	S center_distance_squared = LengthSquared(a.center - b.center);
	return center_distance_squared <= Square(a.radius + b.radius);
}


// Ray-Circle Intersection
// The ray is represented by start + delta * t
// t1 and t2 are the two possible intersection points
// returns false iff the ray misses the sphere (in which case, t1 and t2 are undefined)
template<typename V, typename S> bool GetIntersection(Sphere<V, S> const & sphere, V const & start, V const & delta, S & t1, S & t2)
{
#if 0
//	p = start + delta * t;
//	Length(p - sphere.center) == 
#endif
	
	V sphere_to_start = start - sphere.center;
	S a = LengthSq(delta);
	S half_b = DotProduct(delta, sphere_to_start);
	S c = LengthSq(sphere_to_start) - sphere.radius;
	
	// (Slightly reduced) Quadratic:
	// t = (- half_b (+/-) Sqrt(Square(half_b) - (a * c))) / a
	
	S root = Square(half_b) - a * c;
	if (root < 0)
	{
		return false;
	}
	root = Sqrt(root);
	S inverse_a = Inverse(a);
	
	S p = (- half_b) * inverse_a;
	S q = Sqrt(root) * inverse_a;
	
	t1 = p - q;
	t2 = p + q;

	Assert(t1 <= t2);
	return true;
}

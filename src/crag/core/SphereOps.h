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


template<typename V, typename S> bool Touches(Sphere<V, S> const & sphere, V const & point)
{
	S center_distance_squared = LengthSq(sphere.center - point);
	return center_distance_squared <= Square(sphere.radius);
}

template<typename V, typename S> bool Touches(V const & point, Sphere<V, S> const & sphere)
{
	return Touches(sphere, point);
}

template<typename V, typename S> bool Touches(Sphere<V, S> const & a, Sphere<V, S> const & b)
{
	S center_distance_squared = LengthSquared(a.center - b.center);
	return center_distance_squared <= Square(a.radius + b.radius);
}


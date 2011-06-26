/*
 *  Sphere3.h
 *  Crag
 *
 *  Created by John on 10/31/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Sphere.h"


//////////////////////////////////////////////////////////////////
// 3-dimensional partical specialization of Sphere.
// Often thought of simply as a sphere.

template <typename S> class Sphere<S, 3>
{
public:
	typedef S Scalar;
	typedef Vector<Scalar, 3> Vector;
	
	Sphere() 
	{ }
	
	Sphere(Vector const & c, Scalar r) 
		: center(c)
		, radius(r)
	{ }
	
	Vector center;
	Scalar radius;
};


template<typename S> S Area(Sphere<S, 3> const & s)
{
	return static_cast<S>(PI * 4.) * Square(s.radius);
}

template<typename S> S Volume(Sphere<S, 3> const & s)
{
	return static_cast<S>(PI * 4. / 3.) * Cube(s.radius);
}


//////////////////////////////////////////////////////////////////
// specializations of Sphere3

typedef Sphere<float, 3> Sphere3f;
typedef Sphere<double, 3> Sphere3d;
typedef Sphere<int, 3> Sphere3i;

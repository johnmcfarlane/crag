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
// 2-dimensional partical specialization of Sphere.
// Better known as a circle.

template <typename S> class Sphere<S, 2>
{
public:
	typedef S Scalar;
	typedef Vector<Scalar, 2> Vector;
	
	Sphere() 
	{ }
	
	Sphere(Vector const & c, Scalar r) 
	: center(c)
	, radius(r)
	{ }
	
	Vector center;
	Scalar radius;
};


template<typename S> S Circumference(Sphere<S, 2> const & s)
{
	return static_cast<S>(PI * 2.) * GetRadius();
}

template<typename S> S Area(Sphere<S, 2> const & s)
{
	return static_cast<S>(PI) * Square(GetRadius());
}


//////////////////////////////////////////////////////////////////
// specializations of Sphere2

typedef Sphere<float, 2> Sphere2f;
typedef Sphere<double, 2> Sphere2d;
typedef Sphere<int, 2> Sphere2i;


//////////////////////////////////////////////////////////////////
// circle alias for 2-d sphere

typedef Sphere2f Circle2f;
typedef Sphere2d Circle2d;
typedef Sphere2i Circle2i;

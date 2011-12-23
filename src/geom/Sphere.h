/*
 *  Sphere.h
 *  Crag
 *
 *  Created by John on 10/31/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


// Base class for circles and spheres, i.e. n-spheres.
template<typename S, int N> class Sphere
{
public:
	// types
	typedef S Scalar;
	typedef Vector<Scalar, N> Vector;
	
	// functions
	Sphere() 
	{ }
	
	Sphere(Vector const & c, Scalar r) 
	: center(c)
	, radius(r)
	{ }
	
	template <typename RHS_S>
	Sphere(::Vector<RHS_S, N> const & c, RHS_S r) 
	: center(c)
	, radius(r)
	{ }
	
	template <typename RHS_S>
	Sphere(Sphere<RHS_S, N> const & rhs) 
	: center(rhs.center)
	, radius(rhs.radius)
	{ }

	// variables
	Vector center;
	Scalar radius;
};


// equality operator
template <typename S, int N>
bool operator==(Sphere<S, N> const & lhs, Sphere<S, N> const & rhs)
{
	return lhs.center == rhs.center && lhs.radius == rhs.radius;
}

// inequality operator
template <typename S, int N>
bool operator!=(Sphere<S, N> const & lhs, Sphere<S, N> const & rhs)
{
	return lhs.center != rhs.center && lhs.radius != rhs.radius;
}


// Sphere properties
template<typename S, int N> S SphereDiameter(S radius)
{
	return radius * 2;
}

template<typename S, int N> S SphereArea(S radius);
template<typename S, int N> S SphereVolume(S radius);
template<typename S, int N> S SphereRadiusFromVolume(S volume);

// Area
template<typename S, int N> S Area(Sphere<S, N> const & s)
{
	return SphereArea<S, N>(s.radius);
}

// Volume
template<typename S, int N> S Volume(Sphere<S, N> const & s)
{
	return SphereVolume<S, N>(s.radius);
}


//////////////////////////////////////////////////////////////////
// 3D Sphere property specializations

template<typename S, int N> S SphereArea<S, 3>(S radius)
{
	return static_cast<S>(PI * 4. / 3.) * Cube(radius);
}

template<typename S, int N> S SphereVolume<S, 3>(S radius)
{
	return static_cast<S>(PI * 4. / 3.) * Cube(radius);
}

template<typename S, int N> S SphereRadiusFromVolume<S, 3>(S volume)
{
	return CubeRoot(volume / static_cast<S>(PI * 4. / 3.));
}


//////////////////////////////////////////////////////////////////
// 2D Sphere property specializations

template<typename S> S Circumference(Sphere<S, 2> const & s)
{
	return static_cast<S>(PI * 2.) * s.radius;
}

template<typename S> S Area(Sphere<S, 2> const & s)
{
	return static_cast<S>(PI) * Square(s.radius);
}


//////////////////////////////////////////////////////////////////
// specializations of Sphere2

typedef Sphere<float, 2> Sphere2f;
typedef Sphere<double, 2> Sphere2d;
typedef Sphere<int, 2> Sphere2i;

// circle alias for 2-d sphere
typedef Sphere2f Circle2f;
typedef Sphere2d Circle2d;
typedef Sphere2i Circle2i;


//////////////////////////////////////////////////////////////////
// specializations of Sphere3

typedef Sphere<float, 3> Sphere3f;
typedef Sphere<double, 3> Sphere3d;
typedef Sphere<int, 3> Sphere3i;

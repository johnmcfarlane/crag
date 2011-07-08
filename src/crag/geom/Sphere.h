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
};

// Sphere properties
template<typename S, int N> S SphereDiameter(S radius)
{
	return radius * 2;
}

template<typename S, int N> S SphereArea(S radius);
template<typename S, int N> S SphereVolume(S radius);

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

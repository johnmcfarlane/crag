/*
 *  Sphere.h
 *  Crag
 *
 *  Created by John on 10/31/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

// Base class for circles and spheres, i.e. n-spheres.
template<typename V, typename S> class Sphere
{
public:
	V center;
	S radius;
	
	Sphere() : radius(0) { }
	Sphere(V const & c, S r) : center(c), radius(r) { }
	
	S GetRadius() const {
		return radius;
	}
};


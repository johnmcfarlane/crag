/*
 *  Ray.h
 *  crag
 *
 *  Created by John on 7/27/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Vector3.h"
#include "Matrix4.h"


template <typename S>
class Ray3
{
public:
	typedef S Scalar;
	typedef Vector<S, 3> Vector3;
	
	Ray3() : position(Vector3::Zero()), direction(Vector3::Zero()) 
	{
	}
	
	template <typename S2> Ray3(Ray3<S2> const & rhs) : position(rhs.position), direction(rhs.direction) 
	{
		Verify();
	}
	
	Ray3(Vector3 const & pos, Vector3 const & dir) : position(pos), direction(dir) 
	{
		Verify();
	}
	
	void Verify()
	{ 
		Assert(NearEqual(Length(direction), 1));
	}
	
	// attributes
	Vector3 position;
	Vector3 direction;
};

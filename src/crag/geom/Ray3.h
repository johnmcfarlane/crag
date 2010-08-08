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

#include "Ray.h"


//////////////////////////////////////////////////////////////////
// 3-dimensional partical specialization of Sphere

template <typename S> class Ray<S, 3>
{
public:
	typedef Vector<S, 3> V;
	
	Ray() 
	{
	}
	
	template <typename RHS_S> Ray(Ray<RHS_S, 3> const & rhs) 
		: position(rhs.position)
		, direction(rhs.direction) 
	{
		Verify();
	}
	
	Ray(V const & pos, V const & dir) 
		: position(pos)
		, direction(dir) 
	{
		Verify();
	}
	
	void Verify()
	{ 
		Assert(NearEqual(Length(direction), 1));
	}
	
	// attributes
	V position;
	V direction;
};

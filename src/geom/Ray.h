//
//  Ray.h
//  crag
//
//  Created by John on 8/7/10.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace geom
{
	// A ray is a line with a start and a direction.
	template <typename S, int N> class Ray
	{
	public:
		typedef Vector<S, N> V;
		
		Ray() 
		{
		}
		
		Ray(Ray const & rhs) 
		: position(rhs.position)
		, direction(rhs.direction) 
		{
		}
		
		Ray(V const & pos, V const & dir) 
		: position(pos)
		, direction(dir) 
		{
		}
		
		static Ray Zero() 
		{
			return Ray(V::Zero(), V::Zero()); 
		}
		
		// attributes
		V position;
		V direction;
	};

	// Get a position at some point along ray.
	template <typename S, int N>
	Vector<S, N> Project(Ray<S, N> const & ray, S proportion)
	{
		return ray.position + ray.direction * proportion;
	}
}

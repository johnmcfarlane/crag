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

#if defined(VERIFY)
		void Verify() const
		{
			VerifyObject(position);
			VerifyObject(direction);
		}
#endif
		
		static Ray Zero() 
		{
			return Ray(V::Zero(), V::Zero()); 
		}
		
		// attributes
		V position;
		V direction;
	};

	// returns the length of the given ray
	template<typename S, int N> 
	S LengthSq(Ray<S, N> const & ray)
	{
		return LengthSq(ray.direction);
	}

	// Get a position at some point along ray.
	template <typename S, int N>
	Vector<S, N> Project(Ray<S, N> const & ray, S proportion)
	{
		return ray.position + ray.direction * proportion;
	}

	// catches pointless casts and bypasses them
	template <typename S, int N>
	Ray<S, N> const & Cast(Ray<S, N> const & rhs)
	{
		return rhs;
	}

	// casts between rays of different scalar types
	template <typename LHS_S, typename RHS_S, int N>
	Ray<LHS_S, N> Cast(Ray<RHS_S, N> const & rhs)
	{
		return Ray<LHS_S, N>(Cast<LHS_S>(rhs.position), Cast<LHS_S>(rhs.direction));
	}
	
	template <typename S, int N>
	bool operator==(Ray<S, N> const & lhs, Ray<S, N> const & rhs)
	{
		return lhs.position == rhs.position && lhs.direction == rhs.direction;
	}

	template <typename S, int N>
	bool operator!=(Ray<S, N> const & lhs, Ray<S, N> const & rhs)
	{
		return lhs.position != rhs.position || lhs.direction != rhs.direction;
	}

	// streaming
	template <typename S, int N>
	std::ostream & operator << (std::ostream & out, Ray<S, N> const & ray)
	{
		return out << ray.position << '+' << ray.direction;
	}

	template <typename S, int N>
	std::istream & operator >> (std::istream & in, Ray<S, N> const & ray)
	{
		return in >> ray.position >> '+' >> ray.direction;
	}

	//////////////////////////////////////////////////////////////////
	// Ray<S, N> aliases
	
	typedef Ray<int, 2> Ray2i;
	typedef Ray<float, 2> Ray2f;
	typedef Ray<double, 2> Ray2d;
	
	typedef Ray<int, 3> Ray3i;
	typedef Ray<float, 3> Ray3f;
	typedef Ray<double, 3> Ray3d;
}

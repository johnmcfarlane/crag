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
		typedef S Scalar;
		typedef typename ::geom::Vector<Scalar, N> V;
		
		constexpr Ray() 
		{
		}
		
		constexpr Ray(Ray const & rhs) 
		: position(rhs.position)
		, direction(rhs.direction) 
		{
		}
		
		constexpr Ray(V pos, V dir) 
		: position(pos)
		, direction(dir) 
		{
		}

#if defined(CRAG_VERIFY_ENABLED)
		static void VerifyInvariants(Ray const & object)
		{
			CRAG_VERIFY(object.position);
			CRAG_VERIFY(object.direction);
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

	// returns the magnitude of the given ray
	template<typename S, int N> 
	S MagnitudeSq(Ray<S, N> const &ray)
	{
		return MagnitudeSq(ray.direction);
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
	typename std::enable_if<! std::is_same<LHS_S, RHS_S>::value, Ray<LHS_S, N>>::type Cast(Ray<RHS_S, N> const & rhs)
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

	template <typename S, int N>
	Ray<S, N> operator*=(Ray<S, N> & lhs, S rhs)
	{
		lhs.direction *= rhs;
		return lhs;
	}

	template <typename S, int N>
	Ray<S, N> operator*(Ray<S, N> lhs, S rhs)
	{
		return lhs *= rhs;
	}

	template <typename S, int N>
	Ray<S, N> operator*(S lhs, Ray<S, N> rhs)
	{
		return rhs *= lhs;
	}

	// streaming
	template <typename S, int N>
	std::ostream & operator << (std::ostream & out, Ray<S, N> const & ray)
	{
		return out << ray.position << "<>" << ray.direction;
	}

	template <typename S, int N>
	std::istream & operator >> (std::istream & in, Ray<S, N> const & ray)
	{
		return in >> ray.position >> "<>" >> ray.direction;
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

//
//  Vector.h
//  Planets
//
//  Created by John on May 18, 2010.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace geom 
{
	// This is the un-specialized version of the Vector class.
	template<typename S, int N> class Vector;

	// catches pointless casts and bypasses them
	template <typename S, int N>
	Vector<S, N> const & Cast(Vector<S, N> const & rhs)
	{
		return rhs;
	}

	// common operations
	template <typename S, int N> 
	bool operator != (Vector<S, N> const & lhs, Vector<S, N> const & rhs)
	{
		return ! (lhs == rhs);
	}

	template <typename S, int N> 
	Vector<S, N> operator - (Vector<S, N> lhs, Vector<S, N> const & rhs)
	{
		return lhs -= rhs;
	}

	template <typename S, int N> 
	Vector<S, N> operator + (Vector<S, N> lhs, Vector<S, N> const & rhs)
	{
		return lhs += rhs;
	}

	template <typename S, int N> 
	Vector<S, N> operator * (Vector<S, N> lhs, Vector<S, N> const & rhs)
	{
		return lhs *= rhs;
	}

	template <typename S, int N> 
	Vector<S, N> operator * (Vector<S, N> lhs, S rhs)
	{
		return lhs *= rhs;
	}

	template <typename S, int N> 
	Vector<S, N> operator * (S lhs, Vector<S, N> rhs)
	{
		return rhs *= lhs;
	}

	template <typename S, int N> 
	Vector<S, N> & operator /= (Vector<S, N> & lhs, S rhs)
	{
		return lhs *= (static_cast<S>(1) / rhs);
	}

	template <typename S, int N> 
	Vector<S, N> operator / (Vector<S, N> lhs, Vector<S, N> const & rhs)
	{
		return lhs /= rhs;
	}

	template <typename S, int N>
	Vector<S, N> operator / (Vector<S, N> lhs, S rhs)
	{
		return lhs /= rhs;
	}

	template<typename S, int N>
	S LengthSq(Vector<S, N> const & v)
	{
		return DotProduct(v, v);
	}
}

namespace std
{
	template <typename S, int N>
	S const * begin(geom::Vector<S, N> const & v)
	{
		return & v[0];
	}
	template <typename S, int N>
	S * begin(geom::Vector<S, N> & v)
	{
		return & v[0];
	}
	
	template <typename S, int N>
	S const * end(geom::Vector<S, N> const & v)
	{
		return begin(v) + N;
	}
	template <typename S, int N>
	S * end(geom::Vector<S, N> & v)
	{
		return begin(v) + N;
	}
}

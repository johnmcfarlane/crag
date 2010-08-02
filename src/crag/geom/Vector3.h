/*
 *  Vector3.h
 *  Crag
 *
 *  Created by john on 5/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Vector.h"

#include "core/debug.h"


//////////////////////////////////////////////////////////////////
// 3-dimensional partical specialization of Vector

template<typename S> class Vector<S, 3>
{
public:
	//typedef S S;
	
	Vector() { }
	template<typename RHS_S> Vector(Vector<RHS_S, 3> const & rhs) : x(rhs.x), y(rhs.y), z(rhs.z) { }
	template<typename RHS_S> Vector(RHS_S rhs_x, RHS_S rhs_y, RHS_S rhs_z) : x(rhs_x), y(rhs_y), z(rhs_z) { }

	// Returns vector as a C-style array. Very unsafe. 
	// TODO: Cast as a C++-style fixed-size vector instead.
	S const & operator[](int index) const 
	{
		Assert(index >= 0);
		Assert(index < 3);
		return GetAxes() [index];
	} 
	
	S & operator[](int index) 
	{
		Assert(index >= 0);
		Assert(index < 3);
		return GetAxes() [index];
	} 

	S * GetAxes()
	{
		return reinterpret_cast<S *>(this);
	}

	S const * GetAxes() const
	{
		return reinterpret_cast<S const *>(this);
	}

	static Vector Zero() 
	{
		return Vector(0, 0, 0); 
	}
	
	S x, y, z;
};


template<typename S> bool operator == (Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
{
	return
		lhs.x == rhs.x && 
		lhs.y == rhs.y && 
		lhs.z == rhs.z;
}

template<typename S> bool operator != (Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
{
	return ! (lhs == rhs);
}

template<typename S> inline Vector<S, 3> & operator += (Vector<S, 3> & lhs, Vector<S, 3> const & rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

template<typename S> inline Vector<S, 3> & operator -= (Vector<S, 3> & lhs, Vector<S, 3> const & rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	return lhs;
}

template<typename S> inline Vector<S, 3> & operator *= (Vector<S, 3> & lhs, S rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	return lhs;
}

template<typename S> Vector<S, 3> & operator /= (Vector<S, 3> & lhs, S rhs)
{
	return lhs *= (static_cast<S>(1) / rhs);
}

template<typename S> inline Vector<S, 3> operator - (Vector<S, 3> const & rhs)
{
	return Vector<S, 3>(- rhs.x, - rhs.y, - rhs.z);
}

template<typename S> inline Vector<S, 3> operator - (Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
{
	return ::Vector<S, 3>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

template<typename S> inline Vector<S, 3> operator + (Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
{
	return Vector<S, 3>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

template<typename S> inline Vector<S, 3> operator * (Vector<S, 3> const & lhs, S rhs)
{
	Vector<S, 3> result = lhs;
	return result *= rhs;
}

template<typename S> inline Vector<S, 3> operator * (S lhs, Vector<S, 3> const & rhs)
{
	Vector<S, 3> result = rhs;
	return result *= lhs;
}

template<typename S> Vector<S, 3> operator / (Vector<S, 3> const & lhs, S rhs)
{
	Vector<S, 3> result = lhs;
	return result *= (1.f / rhs);
}

template<typename S> inline S LengthSq(Vector<S, 3> const & v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

template<typename S> S Length(class Vector<S, 3> const & a)
{
	S length_sqaured = LengthSq(a);
	return Sqrt(length_sqaured);
}

template<typename S> S DotProduct(Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename S> Vector<S, 3> CrossProduct(Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
{
	return Vector<S, 3>(
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x);
}

template<typename S> std::ostream & operator << (std::ostream & out, Vector<S, 3> const & v)
{
	return out << v.x << ',' << v.y << ',' << v.z;
}

template<typename S> std::istream & operator >> (std::istream & in, Vector<S, 3> const & v)
{
	return in >> v.x >> ',' >> v.y >> ',' >> v.z;
}


//////////////////////////////////////////////////////////////////
// specializations of Vector3

typedef Vector <float, 3> Vector3f;
typedef Vector <double, 3> Vector3d;
typedef Vector <int, 3> Vector3i;


//////////////////////////////////////////////////////////////////
// TriMod - fn doesn't really have a home but is often found hanging around 3d things.

inline int TriMod(int i)
{
	Assert(i >= 0 && i < 6);
	
	// Alternative 1
	//	return i % 3;
	
	//	// Alternative 2
	//	return (i < 3) ? i : i - 3;
	
	//	// Alternative 3
	static const int table[6] = { 0, 1, 2, 0, 1, 2 };
	return table[i];
}

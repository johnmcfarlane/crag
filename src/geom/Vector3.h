//
//  Vector3.h
//  crag
//
//  Created by john on 5/14/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Vector.h"


//////////////////////////////////////////////////////////////////
// 3-dimensional partical specialization of Vector

template<typename S> class Vector<S, 3>
{
public:
	static int const N = 3;
	
	Vector() 
	{ 
	}

	template<typename RHS_S> Vector(Vector<RHS_S, 3> const & rhs) 
	: x(static_cast<S>(rhs.x))
	, y(static_cast<S>(rhs.y))
	, z(static_cast<S>(rhs.z)) 
	{ }
	
	template<typename RHS_S> Vector(RHS_S rhs_x, RHS_S rhs_y, RHS_S rhs_z) 
	: x(static_cast<S>(rhs_x))
	, y(static_cast<S>(rhs_y))
	, z(static_cast<S>(rhs_z)) 
	{ }
	
	template<typename RHS_S> Vector(RHS_S const * rhs_array) 
	: x(static_cast<S>(rhs_array[0]))
	, y(static_cast<S>(rhs_array[1]))
	, z(static_cast<S>(rhs_array[2])) 
	{ }

	// Returns vector as a C-style array. Very unsafe. 
	S const & operator[](int index) const 
	{
		Assert(index >= 0 && index < N);
		return GetAxes() [index];
	} 
	
	S & operator[](int index) 
	{
		Assert(index >= 0 && index < N);
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
	
	static Vector Max()
	{
		S max_value = std::numeric_limits<S>::max();
		return Vector(max_value, max_value, max_value);
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
	lhs.x /= rhs;
	lhs.y /= rhs;
	lhs.z /= rhs;
	return lhs;
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

// returns v with all axes set to absolute values
template <typename S> 
Vector<S, 3> Abs(Vector<S, 3> const & v)
{
	return Vector<S, 3>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
}

// selects a vector from the plane to which v is perpendicular
template<typename S> Vector<S, 3> Perpendicular(Vector<S, 3> const & v)
{
	typedef Vector<S, 3> Vector;

	Vector a = Abs(v);
	unsigned uyx = Signbit(a.x - a.y);
	unsigned uzx = Signbit(a.x - a.z);
	unsigned uzy = Signbit(a.y - a.z);

	unsigned xm = uyx & uzx;
	unsigned ym = (1^xm) & uzy;
	unsigned zm = 1^(xm | ym);

	Vector p = CrossProduct(v, Vector(xm, ym, zm));
	return p;
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

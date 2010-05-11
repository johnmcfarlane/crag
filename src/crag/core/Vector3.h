/*
 *  Vector3.h
 *  Crag
 *
 *  Created by john on 5/14/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "core/debug.h"


//////////////////////////////////////////////////////////////////
// Vector3 - 3-dimensional vector

template<typename S> class Vector3
{
public:
	typedef S Scalar;
	
	Vector3() { }
	explicit Vector3(Scalar const * array) : x(array[0]), y(array[1]), z(array[2]) { }
	Vector3(Scalar ix, Scalar iy, Scalar iz) : x(ix), y(iy), z(iz) { }
	template<typename I> Vector3(Vector3<I> const & rhs) : x(rhs.x), y(rhs.y), z(rhs.z) { }
	
	Scalar const & operator[](int index) const 
	{
		Assert(index >= 0);
		Assert(index < 3);
		return GetAxes() [index];
	} 
	
	Scalar & operator[](int index) 
	{
		Assert(index >= 0);
		Assert(index < 3);
		return GetAxes() [index];
	} 

	Scalar const * GetAxes() const
	{
		return reinterpret_cast<Scalar const *>(this);
	}

	Scalar * GetAxes() 
	{
		return reinterpret_cast<Scalar *>(this);
	}
	
	static Vector3 Zero() 
	{
		return Vector3(0,0,0); 
	}

	Scalar x, y, z;
};


template<typename Scalar> bool operator == (Vector3<Scalar> const & lhs, Vector3<Scalar> const & rhs)
{
	return
		lhs.x == rhs.x && 
		lhs.y == rhs.y && 
		lhs.z == rhs.z;
}

template<typename Scalar> bool operator != (Vector3<Scalar> const & lhs, Vector3<Scalar> const & rhs)
{
	return ! (lhs == rhs);
}

template<typename Scalar> Vector3<Scalar> & operator += (Vector3<Scalar> & lhs, Vector3<Scalar> const & rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

template<typename Scalar> Vector3<Scalar> & operator -= (Vector3<Scalar> & lhs, Vector3<Scalar> const & rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	return lhs;
}

template<typename Scalar> Vector3<Scalar> & operator *= (Vector3<Scalar> & lhs, Scalar rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	return lhs;
}

template<typename Scalar> Vector3<Scalar> & operator /= (Vector3<Scalar> & lhs, Scalar rhs)
{
	return lhs *= (static_cast<Scalar>(1) / rhs);
}

template<typename Scalar> Vector3<Scalar> operator - (Vector3<Scalar> const & rhs)
{
	return Vector3<Scalar>(- rhs.x, - rhs.y, - rhs.z);
}

template<typename Scalar> Vector3<Scalar> operator - (Vector3<Scalar> const & lhs, Vector3<Scalar> const & rhs)
{
	return Vector3<Scalar>(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

template<typename Scalar> Vector3<Scalar> operator + (Vector3<Scalar> const & lhs, Vector3<Scalar> const & rhs)
{
	return Vector3<Scalar>(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

template<typename Scalar> Vector3<Scalar> operator * (Vector3<Scalar> const & lhs, Scalar rhs)
{
	Vector3<Scalar> result = lhs;
	return result *= rhs;
}

template<typename Scalar> Vector3<Scalar> operator * (Scalar lhs, Vector3<Scalar> const & rhs)
{
	Vector3<Scalar> result = rhs;
	return result *= lhs;
}

template<typename Scalar> Vector3<Scalar> operator / (Vector3<Scalar> const & lhs, Scalar rhs)
{
	Vector3<Scalar> result = lhs;
	return result *= (1.f / rhs);
}

template<typename Scalar> Scalar LengthSq(Vector3<Scalar> const & v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

template<typename Scalar> Scalar Length(class Vector3<Scalar> const & a)
{
	Scalar length_sqaured = LengthSq(a);
	return Sqrt(length_sqaured);
}

template<typename S1, typename S2> S1 DotProduct(Vector3<S1> const & lhs, Vector3<S2> const & rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

template<typename Scalar> Vector3<Scalar> CrossProduct(Vector3<Scalar> const & lhs, Vector3<Scalar> const & rhs)
{
	return Vector3<Scalar>(
		lhs.y * rhs.z - lhs.z * rhs.y,
		lhs.z * rhs.x - lhs.x * rhs.z,
		lhs.x * rhs.y - lhs.y * rhs.x);
}

template<typename Scalar> std::ostream & operator << (std::ostream & out, Vector3<Scalar> const & v)
{
	return out << v.x << ',' << v.y << ',' << v.z;
}

template<typename Scalar> std::istream & operator >> (std::istream & in, Vector3<Scalar> const & v)
{
	return in >> v.x >> ',' >> v.y >> ',' >> v.z;
}


//////////////////////////////////////////////////////////////////
// Vector3f - float specialization of Vector3

typedef Vector3<float> Vector3f;

/*class Vector3f : public Vector3<float>
{
public:
	Vector3f() { }
	explicit Vector3f(float const * array) : Vector3<float>(array) { }
	Vector3f(float ix, float iy, float iz) : Vector3<float>(ix, iy, iz) { }
	template<typename I> Vector3f(Vector3<I> const & rhs) : Vector3<float>(rhs) { }
};*/

/*inline Vector3f & operator /= (Vector3f & lhs, Vector3f const & rhs)
{
	lhs /= rhs;
	return lhs;
}*/


//////////////////////////////////////////////////////////////////
// Vector3d - double specialization of Vector3

typedef Vector3<double> Vector3d;

/*class Vector3d : public Vector3<double>
{
public:
	Vector3d() { }
	template<typename I> Vector3d(I ix, I iy, I iz) : Vector3<double>(ix, iy, iz) { }
	template<typename I> Vector3d(Vector3<I> const & rhs) : Vector3<double>(rhs) { }
};*/


//////////////////////////////////////////////////////////////////
// Vector3i - int specialization of Vector3

typedef Vector3<int> Vector3i;

/*class Vector3i : public Vector3<int>
{
public:
	Vector3i() { }
	template<typename I> Vector3i(I ix, I iy, I iz) : Vector3<int>(ix, iy, iz) { }
	template<typename I> Vector3i(Vector3<I> const & rhs) : Vector3<int>(rhs) { }
};*/


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


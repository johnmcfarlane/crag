/*
 *  Vector4.h
 *  Crag
 *
 *  Created by John on 1/12/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

/*
 *  Vector4.h
 *  Crag
 *
 *  Created by john on 5/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "core/debug.h"

template<typename S> class Vector3;


//////////////////////////////////////////////////////////////////
// Vector4 - 3-dimensional vector plus a W afixed to the end with sticky tape

template<typename S> class Vector4
{
public:
	typedef S Scalar;
	
	Vector4() { }
	explicit Vector4(Scalar const * array) : x(array[0]), y(array[1]), z(array[2]), w(array[3]) { }
	Vector4(Scalar ix, Scalar iy, Scalar iz, Scalar iw) : x(ix), y(iy), z(iz), w(iw) { }
	template<typename I> Vector4(Vector4<I> const & rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) { }
	
	operator Vector3<S> const & () const 
	{
		return reinterpret_cast<Vector3<S> const &>(* this);
	}
	
	Scalar const & operator[](int index) const 
	{
		Assert(index >= 0);
		Assert(index < 4);
		return GetArray() [index];
	} 
	
	Scalar & operator[](int index) 
	{
		Assert(index >= 0);
		Assert(index < 4);
		return GetArray() [index];
	} 
	
	Scalar const * GetArray() const
	{
		return reinterpret_cast<Scalar const *>(this);
	}
	
	Scalar * GetArray() 
	{
		return reinterpret_cast<Scalar *>(this);
	}
	
	static Vector4 Zero() 
	{
		return Vector4(0, 0, 0, 0); 
	}
	
	Scalar x, y, z, w;
};


template<typename Scalar> bool operator == (Vector4<Scalar> const & lhs, Vector4<Scalar> const & rhs)
{
	return
	lhs.x == rhs.x && 
	lhs.y == rhs.y && 
	lhs.z == rhs.z && 
	lhs.w == rhs.w;
}

template<typename Scalar> bool operator != (Vector4<Scalar> const & lhs, Vector4<Scalar> const & rhs)
{
	return ! (lhs == rhs);
}

template<typename Scalar> Vector4<Scalar> & operator += (Vector4<Scalar> & lhs, Vector4<Scalar> const & rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	lhs.w += rhs.w;
	return lhs;
}

template<typename Scalar> Vector4<Scalar> & operator -= (Vector4<Scalar> & lhs, Vector4<Scalar> const & rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	lhs.w -= rhs.w;
	return lhs;
}

/*template<typename Scalar> Vector4<Scalar> & operator *= (Vector4<Scalar> & lhs, Vector4<Scalar> const & rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	lhs.z *= rhs.z;
	lhs.w *= rhs.w;
	return lhs;
}*/

template<typename Scalar> Vector4<Scalar> & operator *= (Vector4<Scalar> & lhs, Scalar rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	lhs.w *= rhs;
	return lhs;
}

template<typename Scalar> Vector4<Scalar> & operator /= (Vector4<Scalar> & lhs, Scalar rhs)
{
	return lhs *= (static_cast<Scalar>(1) / rhs);
}

template<typename Scalar> Vector4<Scalar> operator - (Vector4<Scalar> const & rhs)
{
	return Vector4<Scalar>(- rhs.x, - rhs.y, - rhs.z, - rhs.w);
}

template<typename Scalar> Vector4<Scalar> operator - (Vector4<Scalar> const & lhs, Vector4<Scalar> const & rhs)
{
	Vector4<Scalar> result = lhs;
	return result -= rhs;
}

template<typename Scalar> Vector4<Scalar> operator + (Vector4<Scalar> const & lhs, Vector4<Scalar> const & rhs)
{
	Vector4<Scalar> result = lhs;
	return result += rhs;
}

template<typename Scalar> Vector4<Scalar> operator * (Vector4<Scalar> const & lhs, Vector4<Scalar> const & rhs)
{
	Vector4<Scalar> result = lhs;
	return result *= rhs;
}

template<typename Scalar> Vector4<Scalar> operator * (Vector4<Scalar> const & lhs, Scalar rhs)
{
	Vector4<Scalar> result = lhs;
	return result *= rhs;
}

template<typename Scalar> Vector4<Scalar> operator * (Scalar lhs, Vector4<Scalar> const & rhs)
{
	Vector4<Scalar> result = rhs;
	return result *= lhs;
}

template<typename Scalar> Vector4<Scalar> operator / (Vector4<Scalar> const & lhs, Scalar rhs)
{
	Vector4<Scalar> result = lhs;
	return result *= (1.f / rhs);
}

template<typename Scalar> Scalar LengthSq(Vector4<Scalar> const & v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

template<typename Scalar> Scalar Length(class Vector4<Scalar> const & a)
{
	Scalar length_sqaured = LengthSq(a);
	return Sqrt(length_sqaured);
}

template<typename S1, typename S2> S1 DotProduct(Vector4<S1> const & lhs, Vector4<S2> const & rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}


//////////////////////////////////////////////////////////////////
// Vector4f - float specialization of Vector4

typedef Vector4<float> Vector4f;
/*class Vector4f : public Vector4<float>
{
public:
	Vector4f() { }
	explicit Vector4f(float const * array) : Vector4<float>(array) { }
	Vector4f(float ix, float iy, float iz, float iw) : Vector4<float>(ix, iy, iz, iw) { }
	template<typename I> Vector4f(Vector4<I> const & rhs) : Vector4<float>(rhs) { }
	template<typename I> Vector4f(Vector3<I> const & rhs, I iw = 0) : Vector4<float>(rhs, iw) { }

	operator Vector3f const & () const {
		return reinterpret_cast<Vector3f const &>(* this);
	}
};*/

/*inline Vector4f & operator /= (Vector4f & lhs, Vector4f const & rhs)
{
	lhs /= rhs;
	return lhs;
}*/


//////////////////////////////////////////////////////////////////
// Vector4d - double specialization of Vector4

class Vector4d : public Vector4<double>
{
public:
	Vector4d() { }
	template<typename I> Vector4d(I ix, I iy, I iz, I iw) : Vector4<double>(ix, iy, iz, iw) { }
	template<typename I> Vector4d(Vector4<I> const & rhs) : Vector4<double>(rhs) { }
};


//////////////////////////////////////////////////////////////////
// Vector4i - int specialization of Vector4

class Vector4i : public Vector4<int>
{
public:
	Vector4i() { }
	template<typename I> Vector4i(I ix, I iy, I iz, I iw) : Vector4<int>(ix, iy, iz, iw) { }
	template<typename I> Vector4i(Vector4<I> const & rhs) : Vector4<int>(rhs) { }
};


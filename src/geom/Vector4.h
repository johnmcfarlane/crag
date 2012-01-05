//
//  Vector4.h
//  crag
//
//  Created by john on 5/14/09.
//  Copyright 2009 - 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Vector.h"


//////////////////////////////////////////////////////////////////
// 4-dimensional partical specialization of Vector

template<typename S> class Vector<S, 4>
{
public:
	//typedef S S;
	
	Vector() { }
	template<typename RHS_S> Vector(Vector<RHS_S, 4> const & rhs) : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w) { }

	template<typename RHS_S> Vector(RHS_S rhs_x, RHS_S rhs_y, RHS_S rhs_z, RHS_S rhs_w) 
		: x(static_cast<S>(rhs_x))
		, y(static_cast<S>(rhs_y))
		, z(static_cast<S>(rhs_z))
		, w(static_cast<S>(rhs_w) )
	{ }
	
	// Returns vector as a C-style array. Very unsafe. 
	S const & operator[](int index) const 
	{
		Assert(index >= 0);
		Assert(index < 4);
		return GetAxes() [index];
	} 
	
	S & operator[](int index) 
	{
		Assert(index >= 0);
		Assert(index < 4);
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
		return Vector(0, 0, 0, 0); 
	}
	
	S x, y, z, w;
};



template<typename S> bool operator == (Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
{
	return
	lhs.x == rhs.x && 
	lhs.y == rhs.y && 
	lhs.z == rhs.z && 
	lhs.w == rhs.w;
}

template<typename S> bool operator != (Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
{
	return ! (lhs == rhs);
}

template<typename S> Vector<S, 4> & operator += (Vector<S, 4> & lhs, Vector<S, 4> const & rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	lhs.w += rhs.w;
	return lhs;
}

template<typename S> Vector<S, 4> & operator -= (Vector<S, 4> & lhs, Vector<S, 4> const & rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	lhs.w -= rhs.w;
	return lhs;
}

template<typename S> Vector<S, 4> & operator *= (Vector<S, 4> & lhs, S rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	lhs.z *= rhs;
	lhs.w *= rhs;
	return lhs;
}

template<typename S> Vector<S, 4> & operator /= (Vector<S, 4> & lhs, S rhs)
{
	return lhs *= (static_cast<S>(1) / rhs);
}

template<typename S> Vector<S, 4> operator - (Vector<S, 4> const & rhs)
{
	return Vector<S, 4>(- rhs.x, - rhs.y, - rhs.z, - rhs.w);
}

template<typename S> Vector<S, 4> operator - (Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
{
	Vector<S, 4> result = lhs;
	return result -= rhs;
}

template<typename S> Vector<S, 4> operator + (Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
{
	Vector<S, 4> result = lhs;
	return result += rhs;
}

template<typename S> Vector<S, 4> operator * (Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
{
	Vector<S, 4> result = lhs;
	return result *= rhs;
}

template<typename S> Vector<S, 4> operator * (Vector<S, 4> const & lhs, S rhs)
{
	Vector<S, 4> result = lhs;
	return result *= rhs;
}

template<typename S> Vector<S, 4> operator * (S lhs, Vector<S, 4> const & rhs)
{
	Vector<S, 4> result = rhs;
	return result *= lhs;
}

template<typename S> Vector<S, 4> operator / (Vector<S, 4> const & lhs, S rhs)
{
	Vector<S, 4> result = lhs;
	return result *= (1.f / rhs);
}

template<typename S> S LengthSq(Vector<S, 4> const & v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
}

template<typename S> S DotProduct(Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
{
	return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
}


//////////////////////////////////////////////////////////////////
// specializations of Vector4

typedef Vector<float, 4> Vector4f;
typedef Vector<double, 4> Vector4d;
typedef Vector<int, 4> Vector4i;

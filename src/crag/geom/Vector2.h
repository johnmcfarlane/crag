/*
 *  Vector2.h
 *  Crag
 *
 *  Created by john on 5/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/floatOps.h"


//////////////////////////////////////////////////////////////////
// Vector2 - 2-dimensional vector

template<typename S> class Vector2
{
public:
	typedef S Scalar;
	
	Vector2() { }
	
	Vector2(Scalar ix, Scalar iy) : x(ix), y(iy) { }
	
	template<typename S2> Vector2(Vector2<S2> const & rhs) 
		: x(static_cast<Scalar>(rhs.x)), y(static_cast<Scalar>(rhs.y)) 
	{ 
	}
	
	Scalar const & operator[](int index) const 
	{
		Assert(index >= 0 && index < 2);
		return GetAxes() [index];
	} 
	
	Scalar & operator[](int index) 
	{
		Assert(index >= 0 && index < 2);
		return GetAxes() [index];
	} 

	Scalar const * GetAxes() const
	{
		static_cast<Scalar const *>(this);
	}

	Scalar * GetAxes() 
	{
		static_cast<Scalar *>(this);
	}
	
	static Vector2 Zero() 
	{
		return Vector2(0,0); 
	}
	
	Scalar x, y;
};


template<typename Scalar> bool operator == (Vector2<Scalar> const & lhs, Vector2<Scalar> const & rhs)
{
	return
		lhs.x == rhs.x && 
		lhs.y == rhs.y;
}

template<typename Scalar> bool operator != (Vector2<Scalar> const & lhs, Vector2<Scalar> const & rhs)
{
	return ! (lhs == rhs);
}

template<typename Scalar> Vector2<Scalar> & operator += (Vector2<Scalar> & lhs, Vector2<Scalar> const & rhs)
{
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	return lhs;
}

template<typename Scalar> Vector2<Scalar> & operator -= (Vector2<Scalar> & lhs, Vector2<Scalar> const & rhs)
{
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	return lhs;
}

template<typename Scalar> Vector2<Scalar> & operator *= (Vector2<Scalar> & lhs, Vector2<Scalar> const & rhs)
{
	lhs.x *= rhs.x;
	lhs.y *= rhs.y;
	return lhs;
}

template<typename Scalar1, typename Scalar2> Vector2<Scalar1> & operator *= (Vector2<Scalar1> & lhs, Scalar2 rhs)
{
	lhs.x *= rhs;
	lhs.y *= rhs;
	return lhs;
}

template<typename Scalar> Vector2<Scalar> & operator /= (Vector2<Scalar> & lhs, Scalar rhs)
{
	return lhs *= (static_cast<Scalar>(1) / rhs);
}

template<typename Scalar> Vector2<Scalar> operator - (Vector2<Scalar> const & lhs, Vector2<Scalar> const & rhs)
{
	Vector2<Scalar> result = lhs;
	return result -= rhs;
}

template<typename Scalar> Vector2<Scalar> operator + (Vector2<Scalar> const & lhs, Vector2<Scalar> const & rhs)
{
	Vector2<Scalar> result = lhs;
	return result += rhs;
}

template<typename Scalar> Vector2<Scalar> operator * (Vector2<Scalar> const & lhs, Vector2<Scalar> const & rhs)
{
	Vector2<Scalar> result = lhs;
	return result *= rhs;
}

template<typename Scalar> Vector2<Scalar> operator * (Vector2<Scalar> const & lhs, Scalar rhs)
{
	Vector2<Scalar> result = lhs;
	return result *= rhs;
}

template<typename Scalar> Scalar LengthSq(Vector2<Scalar> const & v)
{
	return v.x * v.x + v.y * v.y;
}

template<typename Scalar> Scalar Length(class Vector2<Scalar> const & a)
{
	Scalar length_sqaured = LengthSq(a);
	return Sqrt(length_sqaured);
}


//////////////////////////////////////////////////////////////////
// Vector2f - float specialization of Vector2

class Vector2f : public Vector2<float>
{
public:
	Vector2f() { }
	Vector2f(float ix, float iy) : Vector2<float>(ix, iy) { }
	//Vector2f(Vector2<float> const & rhs) : Vector2<float>(rhs) { }
	template<typename Scalar> Vector2f(Vector2<Scalar> const & rhs) : Vector2<float>(rhs) { }
};


//////////////////////////////////////////////////////////////////
// Vector2d - double specialization of Vector2

class Vector2d : public Vector2<double>
{
public:
	Vector2d(double ix, double iy) : Vector2<double>(ix, iy) { }
	Vector2d(Vector2<double> const & rhs) : Vector2<double>(rhs) { }
};


//////////////////////////////////////////////////////////////////
// Vector2i - int specialization of Vector2

class Vector2i : public Vector2<int>
{
public:
	Vector2i() { }
	//template<typename Scalar> Vector2i(Scalar ix, Scalar iy) : Vector2<int>(ix, iy) { }
	Vector2i(int ix, int iy) : Vector2<int>(ix, iy) { }
	template<typename Scalar> Vector2i(Vector2<Scalar> const & rhs) : Vector2<int>(rhs) { }
};

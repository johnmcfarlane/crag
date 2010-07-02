/*
 *  VectorUtils.h
 *  Crag
 *
 *  Created by john on 5/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "floatOps.h"


template<typename V> V Normalized(V const & v)
{
	return v * InvSqrt(LengthSq(v));
}

template<typename V> V FastNormalized(V const & v)
{
	return v * FastInvSqrt(LengthSq(v));
}

template<typename V> V & Normalize(V & v)
{
	v *= InvSqrt(LengthSq(v));
	return v;
}

template<typename V> V & FastNormalize(V & v)
{
	v *= FastInvSqrt(LengthSq(v));
	return v;
}

template<typename V> bool SafeNormalize(V & v)
{
	typedef typeof(InvSqrt(LengthSq(v))) S;
	S coefficient = InvSqrt(LengthSq(v));
	
	if (coefficient > 0) {
		v *= coefficient;
		return true;
	}
	else {
		return false;
	}
}

template<typename V> bool FastSafeNormalize(V & v)
{
#if defined(WIN32)
	typedef V::Scalar S;
#else
	typedef typeof(FastInvSqrt(LengthSq(v))) S;
#endif

	S coefficient = FastInvSqrt(LengthSq(v));
	
	if (coefficient > 0) {
		v *= coefficient;
		return true;
	}
	else {
		return false;
	}
}

template<typename S> S TriangleArea(S a, S b, S c)
{
	S p = (a + b + c) * static_cast<S> (.5);	// half perimeter
	S area = Sqrt(p * (p - a) * (p - b) * (p - c));
	return area;
}

// returns un-normalized normal of the triangle
template<typename V> V TriangleNormal(V const & a, V const & b, V const & c)
{
	return CrossProduct(b - a, b - c);
}

// true if a is in between b and c
template<typename V> bool IsInBetween(V const & a, V const & b, V const & c)
{
	V ab = a - b;
	V bc = b - c;	
	if (DotProduct(ab, bc) > 0)
	{
		return false;
	}
	
	V ca = c - a;
	if (DotProduct(bc, ca) > 0)
	{
		return false;
	}
	
	return true;
}

template<typename S, typename V> S TriangleArea(V const & a, V const & b, V const & c)
{
	S ab = Length(a - b);
	S bc = Length(b - c);
	S ca = Length(c - a);
	return TriangleArea(ab, bc, ca);
}

// Distance from triangle, abc to point, p.
// Result is signed. 
template<typename S, typename V> S DistanceToSurface(V const & a, V const & b, V const & c, V const & p) 
{
	V normal = TriangleNormal(a, b, c);
	Normalize(normal);
	
	return DotProduct(normal, p - b);
}

/*
 *  VectorUtils.h
 *  Crag
 *
 *  Created by john on 5/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 *  This file contains useful math functions related to vectors.
 *  Anything vector-specific that can be expressed without reference to its individual members belongs here.
 *	Anything else lives in the file for that particular Vector class, e.g. Vector2.h, Vector3.h etc.
 */

#pragma once

#include "core/floatOps.h"



////////////////////////////////////////////////////////////////////////////////
// Vector Normalization Functions


// Returns a copy of v with unit length. 
// Is undefined when input is zero length.
// Slow but accurate. 
template<typename V> V Normalized(V const & v)
{
	return v * InvSqrt(LengthSq(v));
}

// Returns a copy of v with unit length. 
// Is undefined when input is zero length.
// Fast but inaccurate. 
template<typename V> V FastNormalized(V const & v)
{
	return v * FastInvSqrt(LengthSq(v));
}

// Converts v to unit vector and returns v. 
// Is undefined when input is zero length.
// Slow but accurate. 
template<typename V> V & Normalize(V & v)
{
	v *= InvSqrt(LengthSq(v));
	return v;
}

// Converts v to unit vector and returns v. 
// Is undefined when input is zero length.
// Fast but inaccurate. 
template<typename V> V & FastNormalize(V & v)
{
	v *= FastInvSqrt(LengthSq(v));
	return v;
}

// Converts v to unit vector and returns true. 
// Returns false when input is zero length.
// Slow but accurate. 
template<typename S, int N> bool SafeNormalize(Vector<S, N> & v)
{
	S coefficient = InvSqrt(LengthSq(v));
	
	if (coefficient > 0) {
		v *= coefficient;
		return true;
	}
	else {
		return false;
	}
}

// Converts v to unit vector and returns true. 
// Returns false when input is zero length.
// Slow but accurate. 
template<typename V> bool SafeNormalize(V & v)
{
	// For convenience, guesses what S should be.
	return SafeNormalize<V, V::Scalar>(v);
}

// Converts v to unit vector and returns true. 
// Returns false when input is zero length.
// Fast but inaccurate. 
template<typename S, int N> bool FastSafeNormalize(Vector<S, N> & v)
{
	S coefficient = FastInvSqrt(LengthSq(v));
	
	if (coefficient > 0) {
		v *= coefficient;
		return true;
	}
	else {
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Triangle Functions

// Given the lengths of the sides of a triangle, returns the area of the triangle.
template<typename S> S TriangleArea(S a, S b, S c)
{
	S p = (a + b + c) * static_cast<S> (.5);	// half perimeter
	S area = Sqrt(p * (p - a) * (p - b) * (p - c));
	return area;
}

// Given the positions of the corners of a triangle, returns the area of the triangle.
template<typename S, typename V> S TriangleArea(V const & a, V const & b, V const & c)
{
	S ab = Length(a - b);
	S bc = Length(b - c);
	S ca = Length(c - a);
	return TriangleArea(ab, bc, ca);
}

// Given the positions of the corners of a triangle, returns un-normalized normal of the triangle.
template<typename V> V TriangleNormal(V const & a, V const & b, V const & c)
{
	return CrossProduct(b - a, b - c);
}

// Distance from triangle, abc to point, p.
// Result is signed. 
template<typename S, int N> S DistanceToSurface(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Vector<S, N> const & p) 
{
	Vector<S, N> normal = TriangleNormal(a, b, c);
	Normalize(normal);
	
	return DotProduct(normal, p - b);
}


////////////////////////////////////////////////////////////////////////////////
// Tetrahedron Functions

template<typename S, typename V> S TetrahedronVolume(V const & a, V const & b, V const & c, V const & d)
{
	return Abs(DotProduct((a - d), CrossProduct((b - d), (c - d)))) / 6;
}


////////////////////////////////////////////////////////////////////////////////
// Misc Vector Functions

template<typename S, int N> S Length(class Vector<S, N> const & v)
{
	S length_sqaured = LengthSq(v);
	return Sqrt(length_sqaured);
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


//
//  Magnitude.h
//  crag
//
//  Created by John McFarlane on 9/14/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
//  Functions for determining the magnitude of geometric entities.
//

#pragma once


////////////////////////////////////////////////////////////////////////////////
// Length

template<typename S, int N> S Length(class Vector<S, N> const & v)
{
	S length_sqaured = LengthSq(v);
	return Sqrt(length_sqaured);
}

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
// Area

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


////////////////////////////////////////////////////////////////////////////////
// Volume

template<typename S, typename V> S TetrahedronVolume(V const & a, V const & b, V const & c, V const & d)
{
	return Abs(DotProduct((a - d), CrossProduct((b - d), (c - d)))) / 6;
}


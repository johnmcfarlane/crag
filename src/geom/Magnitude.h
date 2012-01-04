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

// WARNING: Must include one of the VectorN.h headers before this.
// WARNING: Do not directly include Vector.h.


////////////////////////////////////////////////////////////////////////////////
// Length

template<typename S, int N> S Length(Vector<S, N> const & v)
{
	S length_sqaured = LengthSq(v);
	return std::sqrt(length_sqaured);
}

// Returns a copy of v with unit length. 
// Is undefined when input is zero length.
// Slow but accurate. 
template<typename S, int N> Vector<S, N> Normalized(Vector<S, N> const & v)
{
	return v * InvSqrt(LengthSq(v));
}

// Returns a copy of v with given length. 
// Is undefined when input is zero.
// Slow but accurate. 
template<typename S, int N> Vector<S, N> Resized(Vector<S, N> const & v, S length)
{
	return v * (InvSqrt(LengthSq(v)) * length);
}

// Returns a copy of v with unit length. 
// Is undefined when input is zero length.
// Fast but inaccurate. 
template<typename S, int N> Vector<S, N> FastNormalized(Vector<S, N> const & v)
{
	return v * FastInvSqrt(LengthSq(v));
}

// Converts v to unit vector. 
// Is undefined when input is zero length.
// Slow but accurate. 
template<typename S, int N> void Normalize(Vector<S, N> & v)
{
	v *= InvSqrt(LengthSq(v));
}

// Sets v to given length. 
// Is undefined when length is zero.
// Slow but accurate. 
template<typename S, int N> void Resize(Vector<S, N> & v, S length)
{
	v *= (InvSqrt(LengthSq(v)) * length);
}

// Converts v to unit vector and returns v. 
// Is undefined when input is zero length.
// Fast but inaccurate. 
template<typename S, int N> void FastNormalize(Vector<S, N> & v)
{
	v *= FastInvSqrt(LengthSq(v));
}

// Converts v to unit vector and returns true. 
// Returns false when input is zero length.
// Slow but accurate. 
template<typename S, int N> bool SafeNormalize(Vector<S, N> & v)
{
	S coefficient = InvSqrt(LengthSq(v));
	
	if (coefficient > 0) 
	{
		v *= coefficient;
		return true;
	}
	else 
	{
		return false;
	}
}

// Converts v to unit vector and returns true. 
// Returns false when input is zero length.
// Fast but inaccurate. 
template<typename S, int N> bool FastSafeNormalize(Vector<S, N> & v)
{
	S coefficient = FastInvSqrt(LengthSq(v));
	
	if (coefficient > 0) 
	{
		v *= coefficient;
		return true;
	}
	else 
	{
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Area

// Given the lengths of the sides of a triangle, returns the area of the triangle.
template<typename S> S TriangleArea(S a, S b, S c)
{
	S p = (a + b + c) * static_cast<S> (.5);	// half perimeter
	S area = std::sqrt(p * (p - a) * (p - b) * (p - c));
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

// Given the dimensions of a rectangular cuboid, returns the surface area.
template<typename S> S CuboidArea(Vector<S, 3> const & dimensions)
{
	return (dimensions.x * dimensions.y + 
			dimensions.y * dimensions.z + 
			dimensions.z * dimensions.x) * S(2);
}


////////////////////////////////////////////////////////////////////////////////
// Volume

template<typename S, typename V> S TetrahedronVolume(V const & a, V const & b, V const & c, V const & d)
{
	return Abs(DotProduct((a - d), CrossProduct((b - d), (c - d)))) / 6;
}


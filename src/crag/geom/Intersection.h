//
//  Intersection.h
//  crag
//
//  Created by John McFarlane on 9/14/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
//  Functions for determining the intersection between geometric entities.
//

#pragma once

#include "geom/Distance.h"


////////////////////////////////////////////////////////////////////////////////
// Containment tests

// Returns true iff sphere contains point.
template<typename S, int N> bool Contains(Sphere<S, N> const & sphere, Vector<S, N> const & point)
{
	S center_distance_squared = LengthSq(sphere.center - point);
	return center_distance_squared <= Square(sphere.radius);
}

// Returns true iff b is completely inside a.
template<typename S, int N> bool Contains(Sphere<S, N> const & a, Sphere<S, N> const & b)
{
	if (a.radius < b.radius)
	{
		// A is smaller and so cannot contain b.
		return false;
	}
	else
	{
		S center_distance_squared = LengthSq(a.center - b.center);
		return center_distance_squared <= Square(a.radius - b.radius);
	}
}

// Returns true iff b is completely inside a.
// This optimized version assumes that a radius comparison as already been done.
// If the radii have not been compared, use Contains instead.
template<typename S, int N> bool ContainsSmaller(Sphere<S, N> const & a, Sphere<S, N> const & b)
{
	// There's no point running this test is a isn't big enough to contain b in the first place!
	assert(a.radius > b.radius);
	
	S center_distance_squared = LengthSq(a.center - b.center);
	return center_distance_squared <= Square(a.radius - b.radius);
}

// Returns true iff half-space represented by plane, abc, contains point, p.
template<typename S, int N> bool Contains(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Vector<S, N> const & p) 
{
	return DistanceToSurface<S>(a, b, c, p) < 0;
}

// Returns true iff half-space represented by plane, abc, contains point, p.
template<typename S, int N> bool FastContains(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Vector<S, N> const & p) 
{
	return FastDistanceToSurface<S>(a, b, c, p) < 0;
}

// Returns true iff half-space represented by plane, abc, contains sphere, s.
template<typename S, int N> bool Contains(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Sphere<S, N> const & s) 
{
	return DistanceToSurface<S>(a, b, c, s.center) < s.radius;
}

// Returns true iff half-space represented by plane, abc, contains sphere, s.
template<typename S, int N> bool FastContains(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Sphere<S, N> const & s) 
{
	return FastDistanceToSurface<S>(a, b, c, s.center) < s.radius;
}

// Returns true if a is in between b and c.
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


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// Intersection tests

////////////////////////////////////////////////////////////////////////////////
// Line / Line

// Returns true iff lines a and b intersect with one another.
template<typename S, int N> 
bool Intersects(Ray<S, N> const & a, Ray<S, N> const & b)
{
	// In other words, are they not parallel? 
	return DotProduct(a.direction, b.direction) != 0;
}

////////////////////////////////////////////////////////////////////////////////
// Plane / Line

// TODO: New type for plane.

// Returns projection along a at which intersection occurs.
template<typename S, int N> 
S GetIntersection(Ray<S, N> const & plane, Ray<S, N> const & line)
{
	S n = DotProduct(plane.position - line.position, plane.direction);
	if (n == 0)
	{
		return 0;
	}
	
	S d = DotProduct(line.direction, plane.direction);

	return n / d;
}

////////////////////////////////////////////////////////////////////////////////
// Sphere / Sphere

// Returns true iff a and b intersect with one another.
template<typename S, int N> 
bool Intersects(Sphere<S, N> const & a, Sphere<S, N> const & b)
{
	S center_distance_squared = LengthSquared(a.center - b.center);
	return center_distance_squared < Square(a.radius + b.radius);
}

////////////////////////////////////////////////////////////////////////////////
// Sphere / Line

// Returns true iff sphere and line intersect with one another.
// The line is represented by line.position + line.direction * t.
// This is the first half of the following GetIntersection test.
template<typename S, int N> 
bool Intersects(Sphere<S, N> const & sphere, Ray<S, N> const & line)
{
	Vector<S, N> sphere_to_start = line.position - sphere.center;
	S a = LengthSq(line.direction);
	S half_b = DotProduct(line.direction, sphere_to_start);
	S c = LengthSq(sphere_to_start) - Square(sphere.radius);
	
	// (Slightly reduced) Quadratic:
	// t = (- half_b (+/-) Sqrt(Square(half_b) - (a * c))) / a
	
	S root = Square(half_b) - a * c;
	if (root < 0)
	{
		return false;
	}
	
	return true;
}

// Returns true iff sphere and line intersect with one another.
// The line is represented by line.position + line.direction * t.
// The two possible intersection points are t1 and t2.
// If single-point intersection, t1 == t2.
template<typename S, int N> 
bool GetIntersection(Sphere<S, N> const & sphere, Ray<S, N> const & line, S & t1, S & t2)
{
	Vector<S, N> sphere_to_start = line.position - sphere.center;
	S a = LengthSq(line.direction);
	S half_b = DotProduct(line.direction, sphere_to_start);
	S c = LengthSq(sphere_to_start) - Square(sphere.radius);
	
	// (Slightly reduced) Quadratic:
	// t = (- half_b (+/-) Sqrt(Square(half_b) - (a * c))) / a
	
	S root = Square(half_b) - a * c;
	if (root < 0)
	{
		return false;
	}
	root = Sqrt(root);
	S inverse_a = Inverse(a);
	
	S p = (- half_b) * inverse_a;
	S q = root * inverse_a;
	
	t1 = p - q;
	t2 = p + q;
	
	Assert(t1 <= t2);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Convex Polyhedron / Line

// Gets the intersection of a line against a set of planes describing the 
// half-spaces of a convex shape. Iff intersection takes place, returns
// the entry and exit points on the line.
template<typename I, typename S, int N> 
bool GetIntersection(I begin, I end, Ray<S, N> const & line, S & t1, S & t2)
{
	typedef Vector<S, N> Vector;
	typedef Ray<S, N> Ray;
	
	// this is the exit point
	t1 = - std::numeric_limits<S>::max();
	
	// this is the entry point
	t2 = + std::numeric_limits<S>::max();
	
	for (; begin != end; ++ begin)
	{
		Ray const & plane = * begin;
		
		S dp = DotProduct(line.direction, plane.direction);
		if (dp > 0)
		{
			// ray exits shape through plane
			S distance = GetIntersection(plane, line);
			
			if (distance < t2)
			{
				t2 = distance;
			}
			else
			{
				continue;
			}
		}
		else if (dp < 0)
		{
			// ray enters shape through plane
			S distance = GetIntersection(plane, line);
			
			if (distance > t1)
			{
				t1 = distance;
			}
			else
			{
				continue;
			}
		}
		else 
		{
			// algorithm is optimized for general case of lines rarely being
			// paralell to planes.
			continue;
		}
		
		// t1 or t2 have been modified so test intersection holds
		if (t1 < t2)
		{
			continue;
		}
		
		// the ray does not intersect the shape
		return false;
	}
	
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// Sphere / Triangle

// Returns true iff sphere and triangle intersect with one another.
// Original code by David Eberly in Magic. via OPCODE collision library distributed with ODE
template<typename S, int N> bool GetIntersection(Sphere<S, N> const & sphere, Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, S * depth = nullptr)
{
	S mRadius2 = Square(sphere.radius);
	
	Vector<S, N> kDiff;
	S fC;
	
	if (depth == nullptr)
	{
		// Early exit if one of the vertices is inside the sphere
		kDiff = c - sphere.center;
		fC = LengthSq(kDiff);
		if(fC <= mRadius2)	return true;
		
		kDiff = b - sphere.center;
		fC = LengthSq(kDiff);
		if(fC <= mRadius2)	return true;
		
		kDiff = a - sphere.center;
		fC = LengthSq(kDiff);
		if(fC <= mRadius2)	return true;
	}
	else 
	{
		kDiff = a - sphere.center;
		fC = LengthSq(kDiff);
	}
	
	
	// Else do the full distance test
	Vector<S, N> TriEdge0	= b - a;
	Vector<S, N> TriEdge1	= c - a;
	
	//Vector<S, N> kDiff	= a - sphere.center;
	S fA00	= LengthSq(TriEdge0);
	S fA01	= DotProduct(TriEdge0, TriEdge1);
	S fA11	= LengthSq(TriEdge1);
	S fB0	= DotProduct(kDiff, TriEdge0);
	S fB1	= DotProduct(kDiff, TriEdge1);
	//S fC	= LengthSq(kDiff);
	S fDet	= Abs(fA00*fA11 - fA01*fA01);
	S u		= fA01*fB1-fA11*fB0;
	S v		= fA01*fB0-fA00*fB1;
	S SqrDist;
	
	if(u + v <= fDet)
	{
		if(u < 0.0f)
		{
			if(v < 0.0f)  // region 4
			{
				if(fB0 < 0.0f)
				{
					//					v = 0.0f;
					if(-fB0>=fA00)			{ /*u = 1.0f;*/		SqrDist = fA00+2.0f*fB0+fC;	}
					else					{ u = -fB0/fA00;	SqrDist = fB0*u+fC;			}
				}
				else
				{
					//					u = 0.0f;
					if(fB1>=0.0f)			{ /*v = 0.0f;*/		SqrDist = fC;				}
					else if(-fB1>=fA11)		{ /*v = 1.0f;*/		SqrDist = fA11+2.0f*fB1+fC;	}
					else					{ v = -fB1/fA11;	SqrDist = fB1*v+fC;			}
				}
			}
			else  // region 3
			{
				//				u = 0.0f;
				if(fB1>=0.0f)				{ /*v = 0.0f;*/		SqrDist = fC;				}
				else if(-fB1>=fA11)			{ /*v = 1.0f;*/		SqrDist = fA11+2.0f*fB1+fC;	}
				else						{ v = -fB1/fA11;	SqrDist = fB1*v+fC;			}
			}
		}
		else if(v < 0.0f)  // region 5
		{
			//			v = 0.0f;
			if(fB0>=0.0f)					{ /*u = 0.0f;*/		SqrDist = fC;				}
			else if(-fB0>=fA00)				{ /*u = 1.0f;*/		SqrDist = fA00+2.0f*fB0+fC;	}
			else							{ u = -fB0/fA00;	SqrDist = fB0*u+fC;			}
		}
		else  // region 0
		{
			// minimum at interior point
			if(fDet==0.0f)
			{
				//				u = 0.0f;
				//				v = 0.0f;
				SqrDist = std::numeric_limits<S>::max();
			}
			else
			{
				S fInvDet = 1.0f/fDet;
				u *= fInvDet;
				v *= fInvDet;
				SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
			}
		}
	}
	else
	{
		S fTmp0, fTmp1, fNumer, fDenom;
		
		if(u < 0.0f)  // region 2
		{
			fTmp0 = fA01 + fB0;
			fTmp1 = fA11 + fB1;
			if(fTmp1 > fTmp0)
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					//					u = 1.0f;
					//					v = 0.0f;
					SqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					u = fNumer/fDenom;
					v = 1.0f - u;
					SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
			else
			{
				//				u = 0.0f;
				if(fTmp1 <= 0.0f)		{ /*v = 1.0f;*/		SqrDist = fA11+2.0f*fB1+fC;	}
				else if(fB1 >= 0.0f)	{ /*v = 0.0f;*/		SqrDist = fC;				}
				else					{ v = -fB1/fA11;	SqrDist = fB1*v+fC;			}
			}
		}
		else if(v < 0.0f)  // region 6
		{
			fTmp0 = fA01 + fB1;
			fTmp1 = fA00 + fB0;
			if(fTmp1 > fTmp0)
			{
				fNumer = fTmp1 - fTmp0;
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					//					v = 1.0f;
					//					u = 0.0f;
					SqrDist = fA11+2.0f*fB1+fC;
				}
				else
				{
					v = fNumer/fDenom;
					u = 1.0f - v;
					SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
			else
			{
				//				v = 0.0f;
				if(fTmp1 <= 0.0f)		{ /*u = 1.0f;*/		SqrDist = fA00+2.0f*fB0+fC;	}
				else if(fB0 >= 0.0f)	{ /*u = 0.0f;*/		SqrDist = fC;				}
				else					{ u = -fB0/fA00;	SqrDist = fB0*u+fC;			}
			}
		}
		else  // region 1
		{
			fNumer = fA11 + fB1 - fA01 - fB0;
			if(fNumer <= 0.0f)
			{
				//				u = 0.0f;
				//				v = 1.0f;
				SqrDist = fA11+2.0f*fB1+fC;
			}
			else
			{
				fDenom = fA00-2.0f*fA01+fA11;
				if(fNumer >= fDenom)
				{
					//					u = 1.0f;
					//					v = 0.0f;
					SqrDist = fA00+2.0f*fB0+fC;
				}
				else
				{
					u = fNumer/fDenom;
					v = 1.0f - u;
					SqrDist = u*(fA00*u+fA01*v+2.0f*fB0) + v*(fA01*u+fA11*v+2.0f*fB1)+fC;
				}
			}
		}
	}
	
	S AbsSqrDist = Abs(SqrDist);
	if (AbsSqrDist < mRadius2)
	{
		if (depth != nullptr)
		{
			* depth = sphere.radius - Sqrt(AbsSqrDist);
		}
		return true;
	}
	else 
	{
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Surface normal (really needs its own file)

// Given the positions of the corners of a triangle, 
// returns un-normalized normal of the triangle.
template<typename V> V TriangleNormal(V const & a, V const & b, V const & c)
{
	return CrossProduct(b - a, b - c);
}


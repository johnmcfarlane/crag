/*
 *  SphereUtils.h
 *  Crag
 *
 *  Created by John on 1/5/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "VectorOps.h"

#include "core/floatOps.h"


// Point-Circle Intersection Test

template<typename S, int N> bool Contains(Sphere<S, N> const & sphere, Vector<S, N> const & point)
{
	S center_distance_squared = LengthSq(sphere.center - point);
	return center_distance_squared <= Square(sphere.radius);
}

template<typename S, int N> bool IsInside(Vector<S, N> const & point, Sphere<S, N> const & sphere)
{
	return Contains(sphere, point);
}


// Circle-Circle Intersection Test

template<typename S, int N> bool Touches(Sphere<S, N> const & a, Sphere<S, N> const & b)
{
	S center_distance_squared = LengthSquared(a.center - b.center);
	return center_distance_squared <= Square(a.radius + b.radius);
}

// True iff triangle, abc contains sphere, s.
template<typename S, int N> bool Contains(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Sphere<S, N> const & s) 
{
	return DistanceToSurface<S>(a, b, c, s.center) < s.radius;
}

// Sphere-Ray Intersection
// The ray is represented by ray.position + ray.direction * t
// t1 and t2 are the two possible intersection points
// returns false iff the ray misses the sphere (in which case, t1 and t2 are undefined)
template<typename S, int N> bool GetIntersection(Sphere<S, N> const & sphere, Ray<S, N> const & ray, S & t1, S & t2)
{
	Vector<S, N> sphere_to_start = ray.position - sphere.center;
	S a = LengthSq(ray.direction);
	S half_b = DotProduct(ray.direction, sphere_to_start);
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


// Circle-Triangle intersection
// Original code by David Eberly in Magic. via OPCODE collision library distributed with ODE
template<typename S, int N> bool Intersects(Sphere<S, N> const & sphere, Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, S * depth = nullptr)
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

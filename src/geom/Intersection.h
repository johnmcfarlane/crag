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


namespace geom
{
	////////////////////////////////////////////////////////////////////////////////
	// Containment tests

	// Returns true iff sphere contains point.
	template<typename S, int N> bool Contains(Sphere<S, N> const & sphere, Vector<S, N> const & point)
	{
		S center_distance_squared = DistanceSq(sphere.center, point);
		return center_distance_squared <= Squared(sphere.radius);
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
			S center_distance_squared = DistanceSq(a.center, b.center);
			return center_distance_squared <= Squared(a.radius - b.radius);
		}
	}

	// Returns true iff b is completely inside a.
	// This optimized version assumes that a radius comparison as already been done.
	// If the radii have not been compared, use Contains instead.
	template<typename S, int N> bool ContainsSmaller(Sphere<S, N> const & a, Sphere<S, N> const & b)
	{
		// There's no point running this test is a isn't big enough to contain b in the first place!
		assert(a.radius > b.radius);
		
		S center_distance_squared = DistanceSq(a.center, b.center);
		return center_distance_squared <= Squared(a.radius - b.radius);
	}

	// Returns true iff half-space represented by plane, abc, contains point, p.
	template<typename S, int N> bool Contains(Triangle<S, N> const & triangle, Vector<S, N> const & p) 
	{
		return Distance<S>(triangle, p) < 0;
	}

	// Returns true iff half-space represented by plane, abc, contains point, p.
	template<typename S, int N> bool FastContains(Triangle<S, N> const & triangle, Vector<S, N> const & p) 
	{
		return FastDistance<S>(triangle, p) < 0;
	}

	// Returns true iff half-space represented by plane, abc, contains sphere, s.
	template<typename S, int N> bool Contains(Triangle<S, N> const & triangle, Sphere<S, N> const & s) 
	{
		return Distance<S>(triangle, s.center) < s.radius;
	}

	// Returns true iff half-space represented by plane, abc, contains sphere, s.
	template<typename S, int N> bool FastContains(Triangle<S, N> const & triangle, Sphere<S, N> const & s) 
	{
		return FastDistance<S>(triangle, s.center) < s.radius;
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

	// Returns projection along line at which intersection with plane occurs.
	template<typename S, int N> 
	S GetIntersection(Plane<S, N> const & plane, Ray<S, N> const & line)
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
		S center_distance_squared = DistanceSq(a.center, b.center);
		return center_distance_squared < Squared(a.radius + b.radius);
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
		S a = LengthSq(line);
		S half_b = DotProduct(line.direction, sphere_to_start);
		S c = LengthSq(sphere_to_start) - Squared(sphere.radius);
		
		// (Slightly reduced) Quadratic:
		// t = (- half_b (+/-) sqrt(Squared(half_b) - (a * c))) / a
		
		S root = Squared(half_b) - a * c;
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
		S a = LengthSq(line);
		S half_b = DotProduct(line.direction, sphere_to_start);
		S c = LengthSq(sphere_to_start) - Squared(sphere.radius);
		
		// (Slightly reduced) Quadratic:
		// t = (- half_b (+/-) sqrt(Squared(half_b) - (a * c))) / a
		
		S root = Squared(half_b) - a * c;
		if (root < 0)
		{
			return false;
		}
		root = sqrt(root);
		S inverse_a = ::Inverse(a);
		
		S p = (- half_b) * inverse_a;
		S q = root * inverse_a;
		
		t1 = p - q;
		t2 = p + q;
		
		ASSERT(t1 <= t2);
		return true;
	}
}

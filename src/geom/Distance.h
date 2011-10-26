//
//  Distance.h
//  crag
//
//  Created by John McFarlane on 9/14/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
//  Functions for determining the distance between geometric entities.
//

#pragma once


// TODO: Plane class instead of all this a, b, c nonsense.
// TODO: Relational.h


////////////////////////////////////////////////////////////////////////////////
// Surfaces

// Distance from given surface, to point, p.
// Result is signed. If surface.direction isn't unit, result will be proportional.
template<typename S, int N> S DistanceToSurface(Ray<S, N> const & surface, Vector<S, N> const & p) 
{
	return DotProduct(surface.direction, p - surface.position);
}

// Distance from triangle, abc to point, p.
// Result is signed. 
template<typename S, int N> S DistanceToSurface(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Vector<S, N> const & p) 
{
	Vector<S, N> normal = TriangleNormal(a, b, c);
	Normalize(normal);
	
	return DistanceToSurface(Ray<S, N>(c, normal), p);
}

// Distance from triangle, abc to point, p.
// Result is signed. Uses faster, less precise operations.
template<typename S, int N> S FastDistanceToSurface(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Vector<S, N> const & p) 
{
	Vector<S, N> normal = TriangleNormal(a, b, c);
	FastNormalize(normal);
	
	return DistanceToSurface(Ray<S, N>(c, normal), p);
}
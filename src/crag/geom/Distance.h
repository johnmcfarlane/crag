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


////////////////////////////////////////////////////////////////////////////////
// Surfaces

// Distance from given surface, to point, p.
// Result is signed. 
template<typename S, int N> S DistanceToSurface(Vector<S, N> const & surface_point, Vector<S, N> const & surface_normal, Vector<S, N> const & p) 
{
	assert(NearEqual(LengthSq(surface_normal), S(1), S(0.001)));
	
	return DotProduct(surface_normal, p - surface_point);
}

// Distance from triangle, abc to point, p.
// Result is signed. 
template<typename S, int N> S DistanceToSurface(Vector<S, N> const & a, Vector<S, N> const & b, Vector<S, N> const & c, Vector<S, N> const & p) 
{
	Vector<S, N> normal = TriangleNormal(a, b, c);
	Normalize(normal);
	
	return DistanceToSurface(b, normal, p);
}

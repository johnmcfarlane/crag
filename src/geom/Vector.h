//
//  Vector.h
//  Planets
//
//  Created by John on May 18, 2010.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace geom 
{
	// This is the un-specialized version of the Vector class.
	template<typename S, int N> class Vector;

	// catches pointless casts and bypasses them
	template <typename S, int N>
	Vector<S, N> const & Cast(Vector<S, N> const & rhs)
	{
		return rhs;
	}
}

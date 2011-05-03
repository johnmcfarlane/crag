/*
 *  Space.h
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Matrix4.h"
#include "geom/Ray3.h"


namespace axes 
{
	enum Axis
	{
		RIGHT,
		FORWARD,
		UP,
		NUM_AXES
	};
	
	// Returns the given axis from the given matrix.
	template<typename S> inline Vector<S, 3> const & GetAxis(Matrix4<S> const & m, Axis axis)
	{
		return reinterpret_cast<Vector<S, 3> const &> (m.GetRow(axis));
	}
	
	// Returns the versor for the given axis.
	template<typename S> inline Vector<S, 3> GetVersor(Axis axis)
	{
		Assert(axis >= 0 && axis < NUM_AXES);
		
		Vector<S, 3> const versors[NUM_AXES] = 
		{
			{ 1, 0, 0 },
			{ 0, 1, 0 },
			{ 0, 0, 1 },
		};
		
		return versors[axis];
	}
	
	// As opposed to Gl (apparently). 
	template<typename S> inline Matrix4<S> SimToOpenGl()
	{
		return Matrix4<S>(1, 0,  0, 0, 
						  0, 0, -1, 0, 
						  0, 1,  0, 0, 
						  0, 0,  0, 1);
	}
	
	// Converts position/matrix combo to a Ray.
	template<typename S> Ray<S, 3> GetCameraRay(Vector<S, 3> const & pos, Matrix4<S> const & dir)
	{
		return Ray<S, 3>(pos, GetAxis(dir, FORWARD));
	}
	
}

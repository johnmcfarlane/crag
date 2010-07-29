/*
 *  Space.h
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Matrix4.h"
#include "geom/Ray3.h"


// TODO: Space is a terrible name.
namespace space 
{
	enum Axis
	{
		RIGHT,
		FORWARD,
		UP,
		NUM_AXES
	};
	
	// Returns the given axis from the given matrix.
	template<typename S> inline Vector3<S> const & GetAxis(Matrix4<S> const & m, Axis axis)
	{
		return reinterpret_cast<Vector3<S> const &> (m.GetRow(axis));
	}
	
	// Returns the specific axis from the given matrix.
	template<typename S> inline Vector3<S> const & GetRight(Matrix4<S> const & m)
	{
		return GetAxis(m, RIGHT);
	}
	
	template<typename S> inline Vector3<S> const & GetForward(Matrix4<S> const & m)
	{
		return GetAxis(m, FORWARD);
	}
	
	template<typename S> inline Vector3<S> const & GetUp(Matrix4<S> const & m)
	{
		return GetAxis(m, UP);
	}
	
	// Returns the specific global axis.
	template<typename S> inline Vector3<S> GetRight()
	{
		return Vector3<S>(1, 0, 0);
	}
	
	template<typename S> inline Vector3<S> GetForward()
	{
		return Vector3<S>(0, 1, 0);
	}
	
	template<typename S> inline Vector3<S> GetUp()
	{
		return Vector3<S>(0, 0, 1);
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
	template<typename S> Ray3<S> GetCameraRay(Vector3<S> const & pos, Matrix4<S> const & dir)
	{
		return Ray3<S>(pos, GetForward(dir));
	}
	
}

/*
 *  Space.h
 *  Crag
 *
 *  Created by John on 1/24/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "core/Matrix4.h"

// TODO: Space is a terrible name.
namespace Space 
{
	enum Axis
	{
		RIGHT,
		FORWARD,
		UP,
		NUM_AXES
	};
	
	template<typename S> inline Vector3<S> const & GetAxis(Matrix4<S> const & m, Axis axis)
	{
		return reinterpret_cast<Vector3<S> const &> (m.GetRow(axis));
	}
	
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
}

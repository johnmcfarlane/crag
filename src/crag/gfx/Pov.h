/*
 *  Pov.h
 *  Crag
 *
 *  Created by John on 12/8/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "sim/defs.h"


namespace gfx
{
	class Frustum
	{
	public:
		sim::Matrix4 CalcProjectionMatrix() const;
		void SetProjectionMatrix() const;
		
		Vector2i resolution;
		double fov;	// radians
		double near_z;
		double far_z;
	};
	
	// Point of view
	class Pov
	{
	public:
		// types
		typedef sim::Vector3 Vector;
		typedef sim::Matrix4 Matrix;
		
		// functions
		Pov();
		
		void LookAtSphere(Vector const & eye, sim::Sphere3 const & sphere, Vector const & up);
		Matrix GetCameraMatrix() const;
		static Matrix CameraToModelViewMatrix(Matrix const & camera);
		Matrix CalcModelViewMatrix() const;
		
		void SetModelView(Vector const & model_position) const;
		void SetModelView(Vector const & model_position, Matrix const & model_rotation) const;
		
		// variables
		Frustum frustum;
		Vector pos;
		Matrix rot;
	};
}

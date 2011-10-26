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

#include "geom/Transformation.h"


namespace gfx
{
	class Frustum
	{
	public:
		sim::Matrix44 CalcProjectionMatrix() const;
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
		typedef sim::Transformation Transformation;
		
		// functions
		Pov();
		
		Frustum & GetFrustum();
		Frustum const & GetFrustum() const;

		// set/get the camera position/rotation
		void SetTransformation(sim::Transformation const & transformation);
		Transformation const & GetTransformation() const;
		
		Vector GetPosition() const;
		
		//void LookAtSphere(Vector const & eye, sim::Sphere3 const & sphere, Vector const & up);
		
		// set the OpenGl model view matrix for the given model
		void SetModelView(Transformation const & model_transformation) const;
		
	private:
		// variables
		Frustum _frustum;
		Transformation _transformation;
	};
}
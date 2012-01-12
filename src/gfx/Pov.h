//
//  Pov.h
//  crag
//
//  Created by John on 12/8/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "geom/Transformation.h"


namespace gfx
{
	class Frustum
	{
	public:
		Frustum();
		
		Matrix44 CalcProjectionMatrix() const;
		void SetProjectionMatrix() const;
		
		Vector2i resolution;
		double fov;	// radians
		RenderRange depth_range;
	};
	
	// Point of view
	class Pov
	{
	public:
		// functions
		Pov();
		
		Frustum & GetFrustum();
		Frustum const & GetFrustum() const;

		// set/get the camera position/rotation
		void SetTransformation(Transformation const & transformation);
		Transformation const & GetTransformation() const;
		
		Vector3 GetPosition() const;
		
		//void LookAtSphere(Vector const & eye, Sphere3 const & sphere, Vector const & up);
	private:

		// variables
		Frustum _frustum;
		Transformation _transformation;
	};
}

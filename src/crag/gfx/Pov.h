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
	class Frustrum
	{
	public:
		sim::Matrix4 CalcProjectionMatrix() const;

		Vector2i resolution;
		double fov;	// radians
		double near_z;
		double far_z;
	};

	// Point of view
	class Pov
	{
	public:
		void LookAtSphere(sim::Vector3 const & eye, sim::Sphere3 const & sphere, sim::Vector3 const & up);
		sim::Matrix4 GetCameraMatrix(bool translation = true) const;
		static sim::Matrix4 CameraToModelViewMatrix(sim::Matrix4 const & camera);
		sim::Matrix4 CalcModelViewMatrix(bool translation = true) const;
	
		Frustrum frustrum;
		sim::Vector3 pos;
		sim::Matrix4 rot;
	};
}

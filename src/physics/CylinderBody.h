//
//  CylinderBody.h
//  crag
//
//  Created by John on 2014-03-23.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Body.h"


namespace physics
{
	class Engine;

	// a physical body that is cylindrical
	class CylinderBody : public Body
	{
	public:
		CylinderBody(Transformation const & transformation, Vector3 const * velocity, physics::Engine & engine, Scalar radius, Scalar length);
		
		void SetParams(Scalar radius, Scalar length) const;
		void GetParams(Scalar & radius, Scalar & length) const;
		
		virtual void SetDensity(Scalar density) override;
		
	protected:
		virtual bool OnCollision(Body & body, ContactInterface & contact_interface) override;
	};
	
}

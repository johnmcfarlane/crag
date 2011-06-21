/*
 *  SphericalBody.h
 *  crag
 *
 *  Created by John on 6/16/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Body.h"


namespace physics
{
	class Engine;
	
	class SphericalBody : public Body
	{
	public:
		SphericalBody(physics::Engine & physics_engine, bool movable, Scalar radius);
		
		void SetRadius(Scalar radius) const;
		Scalar GetRadius() const;

		virtual void SetDensity(Scalar density);
		
	protected:
		virtual bool OnCollision(Engine & engine, Body & that_body);
		virtual bool OnCollisionWithSphericalBody(Engine & engine, SphericalBody & that_sphere);
	};

}

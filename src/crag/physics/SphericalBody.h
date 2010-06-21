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
	
	class SphericalBody : public Body
	{
		friend class ::physics::Singleton;
		
	protected:
		SphericalBody(sim::Entity & entity, dBodyID body_id, dGeomID geom_id);
		
	public:
		
		void SetRadius(Scalar radius) const;
		Scalar GetRadius() const;

		virtual void SetDensity(Scalar density);
		
	protected:
		virtual bool OnCollision(Body & that_body);
		virtual bool OnCollisionWithSphericalBody(SphericalBody & that_sphere);
	};

}

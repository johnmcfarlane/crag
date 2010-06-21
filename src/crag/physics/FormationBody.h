/*
 *  FormationBody.h
 *  crag
 *
 *  Created by John on 6/20/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "SphericalBody.h"


namespace physics
{
	
	class FormationBody : public SphericalBody
	{
		friend class ::physics::Singleton;
		
		FormationBody(sim::Entity & entity, dBodyID body_id, dGeomID geom_id);
	public:
		
	private:
		virtual bool OnCollision(Body & that_body);
		virtual bool OnCollisionWithSphericalBody(SphericalBody & that_sphere);
	};
	
}

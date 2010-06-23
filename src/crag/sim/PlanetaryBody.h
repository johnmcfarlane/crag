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

#include "physics/SphericalBody.h"


namespace sim
{
	
	class PlanetaryBody : public physics::SphericalBody
	{
		friend class ::physics::Singleton;
		
	public:
		PlanetaryBody(physics::Scalar radius);
		
	private:
		virtual bool OnCollision(Body & that_body);
		virtual bool OnCollisionWithSphericalBody(SphericalBody & that_sphere);
	};
	
}

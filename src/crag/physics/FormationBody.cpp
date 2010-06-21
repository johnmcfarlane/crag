/*
 *  FormationBody.cpp
 *  crag
 *
 *  Created by John on 6/20/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "FormationBody.h"


physics::FormationBody::FormationBody(sim::Entity & entity, dBodyID body_id, dGeomID geom_id)
: SphericalBody(entity, body_id, geom_id)
{
}

bool physics::FormationBody::OnCollision(Body & that_body)
{
	// Rely on other body being a sphere and calling this->OnCollisionWithSphericalBody.
	return false;
}

bool physics::FormationBody::OnCollisionWithSphericalBody(SphericalBody & that_sphere)
{
	return SphericalBody::OnCollisionWithSphericalBody(that_sphere);
}

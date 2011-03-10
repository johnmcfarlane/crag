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

#include "defs.h"
#include "PlanetaryBody.h"
#include "PlanetSphereCollisionFunctor.h"

#include "form/FormationManager.h"
#include "form/scene/SceneThread.h"


////////////////////////////////////////////////////////////////////////////////
// PlanetaryBody members

sim::PlanetaryBody::PlanetaryBody(form::Formation const & init_formation, physics::Scalar init_radius)
: physics::SphericalBody(false, init_radius)
, formation(init_formation)
{
}

bool sim::PlanetaryBody::OnCollision(Body & that_body)
{
	// Rely on other body being a sphere and calling this->OnCollisionWithSphericalBody.
	return false;
}

bool sim::PlanetaryBody::OnCollisionWithSphericalBody(SphericalBody & that_sphere, dGeomID that_geom_id)
{
	sim::Sphere3 sphere(that_sphere.GetPosition(), that_sphere.GetRadius());
	PlanetSphereCollisionFunctor f(formation, geom_id, sphere, that_geom_id);
	
	form::FormationManager::ptr lock = form::FormationManager::GetLock();
	lock->ForEachFormation(f);
	
	return true;
}

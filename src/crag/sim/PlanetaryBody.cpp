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

#include "PlanetaryBody.h"

#include "defs.h"
#include "PlanetSphereCollisionFunctor.h"
#include "Simulation.h"

#include "form/FormationManager.h"
#include "form/scene/SceneThread.h"


////////////////////////////////////////////////////////////////////////////////
// PlanetaryBody members

sim::PlanetaryBody::PlanetaryBody(physics::Engine & physics_engine, form::Formation const & init_formation, physics::Scalar init_radius)
: physics::SphericalBody(physics_engine, false, init_radius)
, formation(init_formation)
{
}

bool sim::PlanetaryBody::OnCollision(physics::Engine & engine, Body & that_body)
{
	// Rely on other body being a sphere and calling this->OnCollisionWithSphericalBody.
	return false;
}

bool sim::PlanetaryBody::OnCollisionWithSphericalBody(physics::Engine & engine, SphericalBody & that_sphere, dGeomID that_geom_id)
{
	// TODO: Fix up collision some time, eh?
	sim::Sphere3 sphere(that_sphere.GetPosition(), that_sphere.GetRadius());
	PlanetSphereCollisionFunctor collision_functor(formation, geom_id, sphere, that_geom_id);
	
	form::ForEachFormationMessage message = { collision_functor };
	form::FormationManager::SendMessage(message, true);
	
	return true;
}

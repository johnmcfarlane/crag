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
#include "Simulation.h"

#include "physics/Engine.h"

#include "form/FormationManager.h"


namespace
{
	
	////////////////////////////////////////////////////////////////////////////////
	// config constants
	
	CONFIG_DEFINE(planet_collision_friction, physics::Scalar, .1);	// coulomb friction coefficient
	CONFIG_DEFINE(planet_collision_bounce, physics::Scalar, .50);

	
	////////////////////////////////////////////////////////////////////////////////
	// IntersectionFunctor - handles individual contact points

	class IntersectionFunctor : public form::FormationManager::IntersectionFunctor
	{
	public:
		IntersectionFunctor(dGeomID object_geom, dGeomID planet_geom)
		: _physics_engine(sim::Simulation::Ref().GetPhysicsEngine())
		{
			ZeroObject(_contact);
			
			_contact.surface.mode = dContactBounce | dContactSlip1 | dContactSlip2;
			
			_contact.surface.mu = planet_collision_friction;
			//_contact.surface.mu2 = 0;
			_contact.surface.bounce = planet_collision_bounce;
			_contact.surface.bounce_vel = .1f;
			//_contact.surface.soft_erp = 0;
			//_contact.surface.soft_cfm = 0;
			//_contact.surface.motion1;
			//_contact.surface.motion2;
			//_contact.surface.motionN;
			//_contact.surface.slip1;
			//_contact.surface.slip2;
			_contact.geom.g1 = object_geom;
			_contact.geom.g2 = planet_geom;
		}
		
	private:
		void operator()(sim::Vector3 const & pos, sim::Vector3 const & normal, sim::Scalar depth) 
		{
			_contact.geom.pos[0] = pos.x;//form::SceneToSim(sim::Scalar(pos.x), _origin.x);
			_contact.geom.pos[1] = pos.y;//form::SceneToSim(sim::Scalar(pos.y), _origin.y);
			_contact.geom.pos[2] = pos.z;//form::SceneToSim(sim::Scalar(pos.z), _origin.z);
			_contact.geom.normal[0] = normal.x;
			_contact.geom.normal[1] = normal.y;
			_contact.geom.normal[2] = normal.z;
			_contact.geom.depth = depth;
			
			_physics_engine.OnContact(_contact);
		}
		
		dContact _contact;
		physics::Engine & _physics_engine;
	};
}	


////////////////////////////////////////////////////////////////////////////////
// PlanetaryBody members

sim::PlanetaryBody::PlanetaryBody(physics::Engine & physics_engine, form::Formation const & init_formation, physics::Scalar init_radius)
: physics::SphericalBody(physics_engine, false, init_radius)
, formation(init_formation)
{
}

form::Formation const & sim::PlanetaryBody::GetFormation() const 
{ 
	return formation; 
}

bool sim::PlanetaryBody::OnCollision(physics::Engine & engine, Body & that_body)
{
	// Rely on other body being a sphere and calling this->OnCollisionWithSphericalBody.
	return false;
}

struct TestSphereCollisionMessage
{
	sim::PlanetaryBody const & _planetary_body;
	physics::SphericalBody const & _spherical_body;
};

bool sim::PlanetaryBody::OnCollisionWithSphericalBody(physics::Engine & engine, SphericalBody & that_sphere)
{
	Sphere3 sphere(that_sphere.GetPosition(), that_sphere.GetRadius());

	dGeomID object_geom = that_sphere.GetGeomId();	
	dGeomID planet_geom = GetGeomId();
	IntersectionFunctor functor(object_geom, planet_geom);

	form::FormationManager & formation_manager = form::FormationManager::Ref();
	formation_manager.ForEachIntersection(sphere, formation, functor);
	
	return true;
}

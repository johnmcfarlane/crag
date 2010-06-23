/*
 *  Planet.cpp
 *  Crag
 *
 *  Created by john on 4/14/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Planet.h"
#include "PlanetShader.h"

#include "sim/PlanetaryBody.h"
#include "physics/Singleton.h"

#include "form/Manager.h"
#include "form/Mesh.h"

#include "core/Random.h"


namespace 
{
	sim::PlanetShaderFactory factory;
}


//////////////////////////////////////////////////////////////////////
// Planet

sim::Planet::Planet(sim::Vector3 const & init_pos, float init_radius, int init_seed)
	: Entity()
	, body(init_radius)
	, formation(factory, init_radius)
{
	body.SetData(this);
	body.SetPosition(init_pos);
	
	Random rnd(init_seed);
	formation.seed = rnd.GetInt();
	formation.SetPosition(init_pos);
	form::Manager::Get().AddFormation(& formation);
}

sim::Planet::~Planet()
{
	form::Manager * formation_manager = form::Manager::GetPtr();
	if (formation_manager != nullptr) {
		form::Manager::Get().RemoveFormation(& formation);
	}
}

bool sim::Planet::IsShadowCatcher() const
{
	return true;
}

void sim::Planet::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	float const density = 1;
	
	Vector3 const & here_pos = body.GetPosition();
	Vector3 there_to_here = here_pos - pos;
	sim::Scalar distance_square = LengthSq(there_to_here);
	sim::Scalar distance = Sqrt(distance_square);
	
	Vector3 direction = there_to_here / distance;

	sim::Scalar radius = formation.GetScale();
	sim::Scalar volume = Cube(radius);
	sim::Scalar mass = volume * density;
	sim::Scalar force = mass / distance_square;

	Vector3 contribution = direction * force;
	gravity += contribution;
}

sim::Scalar sim::Planet::GetBoundingRadius() const
{
//	return formation.GetMaxRadius();
	Assert(false);
	return -1;
}

sim::Vector3 const & sim::Planet::GetPosition() const
{
	return body.GetPosition();
}

#if 0
// this all goes to shit if two planets collide
bool sim::Planet::CustomCollision(PhysicalBody & that_physical) const
{
#if 0
	dGeomID that_geom = that_physical.GetGeom();
	
	Physics & physics = Physics::Get();
	form::Mesh & mesh = physics.GetFormationMesh();
	mesh.ClearPolys();
	
	sim::Sphere3 body(::GetPosition(that_geom) - ::GetPosition(geom), dGeomSphereGetRadius(that_geom));
	
	// Assuming that the other body only ever hits one formation in any one tick.
	// Currently pretty safe as there is only one formation in the simulation!
	Assert(mesh.GetNumPolys() == 0);
	
	{
		sf::Lock lock(form::Manager::mutex);
		
		formation.GenerateCollisionMesh(mesh, body);
		
		physics.OnMeshCollision(that_geom);
	}
	
	return true;
#else
	return false;
#endif
}
#endif

/*#if DUMP
void sim::Planet::Dump(std::ostream & out) const
{
}
#endif*/

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

#include "form/Formation.h"
#include "form/Manager.h"
#include "form/scene/Mesh.h"

#include "core/Random.h"


//////////////////////////////////////////////////////////////////////
// Planet

sim::Planet::Planet(sim::Vector3 const & init_pos, float init_radius_min, float init_radius_max, int init_seed)
: Entity()
, radius_min(init_radius_min)
, radius_max(init_radius_max)
{
	factory = new PlanetShaderFactory(* this);
	
	formation = new form::Formation(* factory);
	Random rnd(init_seed);
	formation->seed = rnd.GetInt();
	formation->SetPosition(init_pos);
	form::Manager::Get().AddFormation(formation);
	
	// TODO: Is that an accurate average radius? Shouldn't it be Logarithmic?
	body = new PlanetaryBody(* formation, radius_max);
	body->SetPosition(init_pos);
}

sim::Planet::~Planet()
{
	form::Manager * formation_manager = form::Manager::GetPtr();
	if (formation_manager != nullptr) {
		form::Manager::Get().RemoveFormation(formation);
	}
}

void sim::Planet::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	float const density = 1;
	
	Vector3 const & here_pos = body->GetPosition();
	Vector3 there_to_here = here_pos - pos;
	sim::Scalar distance_square = LengthSq(there_to_here);
	sim::Scalar distance = Sqrt(distance_square);
	
	Vector3 direction = there_to_here / distance;

	sim::Scalar radius = GetAverageRadius();
	sim::Scalar volume = Cube(radius);
	sim::Scalar mass = volume * density;
	sim::Scalar force = mass / distance_square;

	Vector3 contribution = direction * force;
	gravity += contribution;
}

bool sim::Planet::GetRenderRange(Ray3 const & camera_ray, double * range) const
{
	// TODO: Reduce the max when it's obscured.
	Scalar distance_squared = LengthSq(camera_ray.position - GetPosition());
	
	// Is camera inside the planet?
	Scalar radius_min_squared = Square(radius_min);
	if (distance_squared < radius_min_squared)
	{
		Scalar distance = Sqrt(distance_squared);
		range[0] = radius_min - distance;
		range[1] = distance + radius_max; 
		return true;
	}

	// Is camera outside the entire planet?
	Scalar radius_max_squared = Square(radius_max);
	if (distance_squared > radius_max_squared)
	{
		Scalar distance = Sqrt(distance_squared);
		range[0] = distance - radius_max;
	}
	else 
	{
		range[0] = 0;
	}
	

	// The furthest you might ever be able too see of this planet
	// is a ray that skims the sphere of radius_min
	// and then hits the sphere of radius_max.
	// For some reason, those two distances appear to be very easy to calculate. 
	// (Famous last words.)
	Scalar a = Sqrt(distance_squared - radius_min_squared);
	Scalar b = Sqrt(radius_max_squared - radius_min_squared);
	range[1] = a + b;
	
	return true;
}

sim::Scalar sim::Planet::GetAverageRadius() const
{
	return (radius_min + radius_max) * .5;
}

sim::Vector3 const & sim::Planet::GetPosition() const
{
	return body->GetPosition();
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
		
		formation->GenerateCollisionMesh(mesh, body);
		
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

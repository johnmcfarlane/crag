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
#include "Physics.h"
#include "PhysicalBody.h"
#include "PlanetShader.h"

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
	, formation(factory, init_radius)
#if SEA
	, sea(init_radius)
#endif
{
	Physics & physics = Physics::Get();

	geom = physics.CreateSphere(* this, 1, init_radius);
	dGeomSetPosition(geom, init_pos.x, init_pos.y, init_pos.z);
	
	Random rnd(init_seed);
	formation.seed = rnd.GetInt();
	formation.SetPosition(init_pos);
	form::Manager::Get().AddFormation(& formation);
	
#if SEA
	sea.Init(rnd.GetInt());
	sea.SetPosition(init_pos);
	form::Manager::AddFormation(sea);
#endif
	
	shadow_map.Init();
	Bind(& shadow_map);
	shadow_map.Resize(1024, 1024);
}

sim::Planet::~Planet()
{
	form::Manager * formation_manager = form::Manager::GetPtr();
	if (formation_manager != nullptr) {
		form::Manager::Get().RemoveFormation(& formation);
	}

	Physics & physics = Physics::Get();
	physics.Destroy(geom);
}

bool sim::Planet::IsShadowCatcher() const
{
	return true;
}

void sim::Planet::Tick()
{
	Vector3 const & pos = sim::GetPosition(geom);
//	Matric4f const & matrix = 
//	Vector3 const & camera_pos = Graphics::GetCameraPos();
//	Vector3 const & camera_dir = Graphics::GetCameraDir();
	
	formation.SetPosition(pos);
//	land.SetCameraPos(camera_pos, camera_dir);
#if SEA
	sea.SetPosition(pos);
//	sea.SetCameraPos(camera_pos, camera_dir);
#endif

	// Update the planet's radius so it collides with anything it /might/ be touching.
/*	float max_radius = land.GetMaxRadius();
	dGeomSphereSetRadius(geom, max_radius);*/
}

void sim::Planet::GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const
{
	float const density = 1;
	
	Vector3 there_to_here = sim::GetPosition(geom) - pos;
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

/*int CountDescendants(FormationNode & node)
{
	int num_nodes = 0;
	
	for (int i = 0; i < 4; ++ i)
	{
		FormationNode * child = node.GetChild(i);
		if (child != nullptr)
		{
			num_nodes += CountDescendants(* child) + 1;
		}
	}
	
	return num_nodes;
}*/

float sim::Planet::GetBoundingRadius() const
{
//	return formation.GetMaxRadius();
	Assert(false);
	return -1;
}

sim::Vector3 const & sim::Planet::GetPosition() const
{
	return sim::GetPosition(geom);
}

// this all goes to shit if two planets collide
bool sim::Planet::CustomCollision(PhysicalBody & that_physical) const
{
/*	dGeomID that_geom = that_physical.GetGeom();
	
	Physics & physics = Physics::Get();
	form::Mesh & mesh = physics.GetFormationMesh();
	mesh.ClearPolys();
	
	sim::Sphere3 sphere(::GetPosition(that_geom) - ::GetPosition(geom), dGeomSphereGetRadius(that_geom));
	
	// Assuming that the other body only ever hits one formation in any one tick.
	// Currently pretty safe as there is only one formation in the simulation!
	Assert(mesh.GetNumPolys() == 0);
	
	{
		sf::Lock lock(form::Manager::mutex);
		
		formation.GenerateCollisionMesh(mesh, sphere);
		
		physics.OnMeshCollision(that_geom);
	}*/
	
	return true;
}

/*#if DUMP
void sim::Planet::Dump(std::ostream & out) const
{
}
#endif*/

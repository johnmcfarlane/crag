//
//  physics::Engine.cpp
//  crag
//
//  Created by John on 6/19/10.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Engine.h"
#include "Body.h"

#include "form/scene/Scene.h"

#include "core/ConfigEntry.h"

#if ! defined(NDEBUG)
//#define DEBUG_CONTACT
#endif

#if defined(DEBUG_CONTACT)
#include "gfx/Debug.h"
#endif


using namespace physics;

namespace 
{
	CONFIG_DEFINE (collisions, bool, true);
}
CONFIG_DEFINE (collisions_parallelization, bool, true);


//////////////////////////////////////////////////////////////////////
// physics::Engine members

Engine::Engine()
: world(dWorldCreate())
, space(dSimpleSpaceCreate(0))
, contact_joints(dJointGroupCreate(0))
, _formation_scene(ref(new form::Scene(512, 512)))
, _contacts(65536)
{
	dInitODE2(0);
}

Engine::~Engine()
{
	delete & _formation_scene;
	dSpaceDestroy(space);
	dWorldDestroy(world);
	dJointGroupDestroy(contact_joints);
	dCloseODE();
}

form::Scene & Engine::GetScene()
{
	return _formation_scene;
}

form::Scene const & Engine::GetScene() const
{
	return _formation_scene;
}

dBodyID Engine::CreateBody() const
{
	return dBodyCreate(world);
}

dGeomID Engine::CreateBox(Vector3 const & dimensions) const
{
	return dCreateBox(space, dimensions.x, dimensions.y, dimensions.z);
}

dGeomID Engine::CreateSphere(Scalar radius) const
{
	return dCreateSphere(space, radius);
}

void Engine::Attach(Body const & body1, Body const & body2)
{
	dJointID joint_id = dJointCreateBall(world, 0);
	if (joint_id == nullptr)
	{
		ASSERT(false);
		return;
	}
	
	physics::Attach(joint_id, body1, body2);
	dJointSetFixed (joint_id);
}

void Engine::Tick(double delta_time)
{
	_formation_scene.Tick();
	
	if (collisions)
	{
		// Detect / represent all collisions.
		CreateCollisions();
		CreateJoints();
		
		// Tick physics (including acting upon collisions).
		dWorldQuickStep (world, delta_time);
		//dWorldStep (world, delta_time);
		
		// Remove this tick's collision data.
		DestroyJoints();
		DestroyCollisions();
	}
	else
	{
		// No collision checking, so just tick physics.
		dWorldQuickStep (world, delta_time);
	}
}

void Engine::ToggleCollisions()
{
	collisions = ! collisions;
}

void Engine::CreateCollisions()
{
	// This basically calls a callback for all the geoms that are quite close.
	dSpaceCollide(space, reinterpret_cast<void *>(this), OnNearCollisionCallback);
}

void Engine::CreateJoints()
{
	for (ContactVector::const_iterator i = _contacts.begin(); i != _contacts.end(); ++ i)
	{
		dContact const & contact = * i;
		
		dBodyID body1 = dGeomGetBody(contact.geom.g1);
		dBodyID body2 = dGeomGetBody(contact.geom.g2);
		
		// body sanity tests
		ASSERT(body1 != body2);
		ASSERT(body1 == nullptr || dBodyGetWorld(body1) == world);
		ASSERT(body2 == nullptr || dBodyGetWorld(body2) == world);	
		
		dJointID c = dJointCreateContact (world, contact_joints, & contact);
		dJointAttach (c, body1, body2);
	}
}

void Engine::DestroyJoints()
{
	dJointGroupEmpty(contact_joints);
}

void Engine::DestroyCollisions()
{
	_contacts.clear();
}

// Called by ODE when geometry collides. In the case of planets, 
// this means the outer shell of the planet.
void Engine::OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2)
{
	physics::Engine & engine = * reinterpret_cast<physics::Engine *>(data);
	
	Body & body1 = ref(reinterpret_cast<Body *>(dGeomGetData(geom1)));
	Body & body2 = ref(reinterpret_cast<Body *>(dGeomGetData(geom2)));
	
	if (body1.OnCollision(engine, body2))
	{
		return;
	}
	
	if (body2.OnCollision(engine, body1))
	{
		return;
	}
	
	engine.OnUnhandledCollision(geom1, geom2);
}

// This is the default handler. It leaves ODE to deal with it. 
void Engine::OnUnhandledCollision(dGeomID geom1, dGeomID geom2)
{
	// No reason not to keep this nice and high; it's on the stack.
	int const max_contacts_per_collision = 1;
	dContactGeom contact_geoms [max_contacts_per_collision];
	
	int num_contacts = dCollide (geom1, geom2, max_contacts_per_collision, contact_geoms, sizeof(dContactGeom));
	if (num_contacts == 0)
	{
		return;
	}
	
	// Time to increase max_num_contacts?
	ASSERT (num_contacts <= max_contacts_per_collision);
	
	dContact * contacts = _contacts.grow(num_contacts);
	
	for (int index = 0; index < num_contacts; ++ index)
	{
		dContactGeom & contact_geom = contact_geoms[index];
		dContact & contact = contacts[index];
		
		contact.geom = contact_geom;
		
		// init the surface member of the contact
		dSurfaceParameters & surface = contact.surface;
		surface.mode = dContactBounce | dContactSoftCFM;
		surface.mu = 1;
		surface.bounce = .5f;
		surface.bounce_vel = .1f;
		
		contact.geom.g1 = geom1;
		contact.geom.g2 = geom2;
	}
}

void Engine::OnContact(dContact const & contact)
{
	// geometry sanity tests
	ASSERT(contact.geom.g1 != contact.geom.g2);
	ASSERT(contact.geom.depth >= 0);

	//std::cout << contact.geom.depth << ' ' << contact.geom.normal[0] << ',' << contact.geom.normal[1] << ',' << contact.geom.normal[2] << '\n';

#if defined(DEBUG_CONTACT)
	Vector3 pos(contact.geom.pos[0], contact.geom.pos[1], contact.geom.pos[2]);
	Vector3 normal(contact.geom.normal[0], contact.geom.normal[1], contact.geom.normal[2]);
	gfx::Debug::AddLine(pos, pos + normal * contact.geom.depth * 100.);
	std::cout << pos << ' ' << normal << ' ' << contact.geom.depth << '\n';
#endif

	_contacts.push_back(contact);
}

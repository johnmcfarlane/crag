/*
 *  Engine.cpp
 *  crag
 *
 *  Created by John on 6/19/10.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Engine.h"
#include "SphericalBody.h"

#include "form/FormationManager.h"

#include "smp/for_each.h"
#include "smp/Lock.h"

#include "core/ConfigEntry.h"


using namespace physics;

namespace 
{
	CONFIG_DEFINE (collisions, bool, true);
}
CONFIG_DEFINE (collisions_parallelization, bool, true);


//////////////////////////////////////////////////////////////////////
// physics::Engine members

physics::Engine::Engine()
: world(dWorldCreate())
, space(dSimpleSpaceCreate(0))
, contact_joints(dJointGroupCreate(0))
, _deferred_collisions(4096)
, _contacts(65536)
{
	dInitODE2(0);
}

physics::Engine::~Engine()
{
	dSpaceDestroy(space);
	dWorldDestroy(world);
	dJointGroupDestroy(contact_joints);
	dCloseODE();
}

dBodyID physics::Engine::CreateBody() const
{
	return dBodyCreate(world);
}

dGeomID physics::Engine::CreateBox(Vector3 const & dimensions) const
{
	return dCreateBox(space, dimensions.x, dimensions.y, dimensions.z);
}

dGeomID physics::Engine::CreateSphere(Scalar radius) const
{
	return dCreateSphere(space, radius);
}

void physics::Engine::Tick(double delta_time)
{
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

void physics::Engine::ToggleCollisions()
{
	collisions = ! collisions;
}

void physics::Engine::CreateCollisions()
{
	// This basically calls a callback for all the geoms that are quite close.
	dSpaceCollide(space, reinterpret_cast<void *>(this), OnNearCollisionCallback);
	
	ProcessDeferredCollisions();	
}

void physics::Engine::CreateJoints()
{
	for (ContactVector::const_iterator i = _contacts.begin(); i != _contacts.end(); ++ i)
	{
		dContact const & contact = * i;
		
		dBodyID body1 = dGeomGetBody(contact.geom.g1);
		dBodyID body2 = dGeomGetBody(contact.geom.g2);
		
		// body sanity tests
		Assert(body1 != body2);
		Assert(body1 == nullptr || dBodyGetWorld(body1) == world);
		Assert(body2 == nullptr || dBodyGetWorld(body2) == world);	
		
		dJointID c = dJointCreateContact (world, contact_joints, & contact);
		dJointAttach (c, body1, body2);
	}
}

void physics::Engine::ProcessDeferredCollisions()
{
	if (_deferred_collisions.empty())
	{
		return;
	}
	
	form::FormationManager & formation_manager = form::FormationManager::Daemon::Ref();
	
	formation_manager.LockTree();
	smp::scheduler::Complete(_deferred_collisions, 1);
	formation_manager.UnlockTree();
	
	_deferred_collisions.clear();
}

void physics::Engine::DestroyJoints()
{
	dJointGroupEmpty(contact_joints);
}

void physics::Engine::DestroyCollisions()
{
	_contacts.clear();
}

// Called by ODE when geometry collides. In the case of planets, 
// this means the outer shell of the planet.
void physics::Engine::OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2)
{
	Engine & engine = * reinterpret_cast<Engine *>(data);
	
	Body & body1 = ref(reinterpret_cast<Body *>(dGeomGetData(geom1)));
	Body & body2 = ref(reinterpret_cast<Body *>(dGeomGetData(geom2)));
	
	Vector3 p1 = body1.GetPosition();
	Vector3 p2 = body2.GetPosition();
	
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
void physics::Engine::OnUnhandledCollision(dGeomID geom1, dGeomID geom2)
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
	Assert (num_contacts <= max_contacts_per_collision);
	
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

void physics::Engine::OnContact(dContact const & contact)
{
	// geometry sanity tests
	Assert(contact.geom.g1 != contact.geom.g2);
	Assert(contact.geom.depth > 0);
	
	_contacts.push_back(contact);
}

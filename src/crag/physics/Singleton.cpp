/*
 *  Singleton.cpp
 *  crag
 *
 *  Created by John on 6/19/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "pch.h"

#include "Singleton.h"
#include "SphericalBody.h"


//////////////////////////////////////////////////////////////////////
// physics::Singleton members

physics::Singleton::Singleton()
: world(dWorldCreate())
, space(dSimpleSpaceCreate(0))
, contact_joints(dJointGroupCreate(0))
{
	dInitODE2(0);
}

physics::Singleton::~Singleton()
{
	dSpaceDestroy(space);
	dWorldDestroy(world);
	dJointGroupDestroy(contact_joints);
	dCloseODE();
}

void physics::Singleton::Tick(double delta_time)
{
	// Detect / represent all collisions.
	CreateCollisions();
	
	// Tick physics (including acting upon collisions).
	dWorldQuickStep (world, delta_time);
	
	// Remove this tick's collision data.
	DestroyCollisions();
}

void physics::Singleton::CreateCollisions()
{
	// This basically calls a callback for all the geoms that are quite close.
	dSpaceCollide(space, reinterpret_cast<void *>(this), OnNearCollisionCallback);
}

void physics::Singleton::DestroyCollisions()
{
	dJointGroupEmpty(contact_joints);
}

void physics::Singleton::OnNearCollisionCallback (void *data, dGeomID geom1, dGeomID geom2)
{
#if ! defined(NDEBUG)
	Singleton & ode_singleton = * reinterpret_cast<Singleton *>(data);
	Singleton & singleton = Get();
	Assert(& ode_singleton == & singleton);
#endif
	
	Body & body1 = ref(reinterpret_cast<Body *>(dGeomGetData(geom1)));
	Body & body2 = ref(reinterpret_cast<Body *>(dGeomGetData(geom2)));
	
	if (body1.OnCollision(body2))
	{
		return;
	}
	
	if (body2.OnCollision(body1))
	{
		return;
	}
	
	// Nothing handled the collision between body1 and body2.
	Assert(false);
}

void physics::Singleton::OnCollision(dGeomID geom1, dGeomID geom2)
{
	// No reason not to keep this nice and high; it's on the stack.
	// TODO: If cost of loop below gets too much, this array can be made a permanent member of this object and much of the init can be skipped.
	int const max_num_contacts = 128;
	dContact contacts [max_num_contacts];
	
	int num_contacts = dCollide (geom1, geom2, max_num_contacts, & contacts[0].geom, sizeof(* contacts));
	if (num_contacts == 0)
	{
		return;
	}
	
	// Time to increase max_num_contacts?
	Assert (num_contacts < max_num_contacts);
	
	dContact const * const end = contacts + num_contacts;
	for (dContact * it = contacts; it != end; ++ it)
	{
		// init the surface member of the contact
		dSurfaceParameters & surface = it->surface;
		surface.mode = dContactBounce | dContactSoftCFM;
		surface.mu = 1;				// used (by default)
		//surface.mu2 = 0;
		surface.bounce = .5f;		// used
		surface.bounce_vel = .1f;	// used
		//surface.soft_erp = 0;
		//surface.soft_cfm = 0.001;
		//surface.motion1 = 0;
		//surface.motion2 = 0;
		//surface.motionN = 0;
		//surface.slip1 = 0;
		//surface.slip2 = 0;
		
		//it->fdir1[0] = 0;
		//it->fdir1[1] = 0;
		//it->fdir1[2] = 0;
		//it->fdir1[3] = 0;
		
		OnContact(* it, geom1, geom2);
	}
}

void physics::Singleton::OnContact(dContact const & contact, dGeomID geom1, dGeomID geom2)
{
	dJointID c = dJointCreateContact (world, contact_joints, & contact);
	dBodyID body1 = dGeomGetBody(geom1);
	dBodyID body2 = dGeomGetBody(geom2);
	dJointAttach (c, body1, body2);
}

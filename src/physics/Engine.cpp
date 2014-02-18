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

#include "RayCast.h"
#include "SphericalBody.h"

#include "core/ConfigEntry.h"
#include "core/Roster.h"
#include "core/Statistics.h"

#if ! defined(NDEBUG)
//#define DEBUG_CONTACTS
#endif

#if defined(DEBUG_CONTACTS)
#include "gfx/Debug.h"
#endif

#include <ode/ode.h>

using namespace physics;

namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// config constants
	
	CONFIG_DEFINE(collisions, bool, true);
	CONFIG_DEFINE(contact_surface_friction, physics::Scalar, 1.);	// coulomb friction coefficient
	CONFIG_DEFINE(contact_surface_bounce, physics::Scalar, .5);
	CONFIG_DEFINE(contact_surface_bounce_velocity, physics::Scalar, .1f);

	STAT (num_contacts, int, .15f);

#if ! defined(NDEBUG)
	void odeMessageFunction (int errnum, const char *msg, va_list ap)
	{
		constexpr std::size_t buffer_size = 4096;
		char buffer[buffer_size];
		vsnprintf(buffer, buffer_size, msg, ap);
		DEBUG_BREAK("ODE#%d: %s", errnum, buffer); 
	}
#endif

	void OnCastRayCollision(void *, CollisionHandle body_handle, CollisionHandle ray_handle)
	{
		ASSERT(dGeomGetClass(ray_handle) == dRayClass);

		void * body_data = dGeomGetData (body_handle);
		Body & body = ref(static_cast<Body *>(body_data));
	
		void * ray_data = dGeomGetData (ray_handle);
		RayCast & ray_cast = ref(static_cast<RayCast *>(ray_data));
	
		body.OnCollisionWithRay(ray_cast);
	}

	void OnSphereCollision(void * data, CollisionHandle body_handle, CollisionHandle sphere_handle)
	{
		//ASSERT(dGeomGetClass(body_handle) == dSphereClass);
		
		auto & contact_interface = ref(static_cast<ContactInterface *>(data));

		auto body_data = dGeomGetData (body_handle);
		auto & body = ref(static_cast<Body *>(body_data));
	
		auto sphere_data = dGeomGetData (sphere_handle);
		auto & sphere = ref(static_cast<Body *>(sphere_data));
	
		sphere.OnCollision(body, contact_interface);
	}
}
CONFIG_DEFINE (collisions_parallelization, bool, true);


//////////////////////////////////////////////////////////////////////
// physics::Engine members

Engine::Engine()
: world(dWorldCreate())
, space(dSimpleSpaceCreate(0))
, contact_joints(dJointGroupCreate(0))
, _pre_tick_roster(ref(new core::locality::Roster))
, _post_tick_roster(ref(new core::locality::Roster))
{
	// init ODE error handling
#if ! defined(NDEBUG)
	dSetErrorHandler(odeMessageFunction);
	dSetDebugHandler(odeMessageFunction);
	dSetMessageHandler(odeMessageFunction);
#endif

	// init ODE
	dInitODE();

	// init _contact
	ZeroObject(_contact);
	_contact.surface.mode = dContactBounce;
	_contact.surface.mu = contact_surface_friction;
	_contact.surface.bounce = contact_surface_bounce;
	_contact.surface.bounce_vel = contact_surface_bounce_velocity;
}

Engine::~Engine()
{
	dSpaceDestroy(space);
	dWorldDestroy(world);
	dJointGroupDestroy(contact_joints);
	dCloseODE();
}

core::locality::Roster & Engine::GetPreTickRoster()
{
	return _pre_tick_roster;
}

core::locality::Roster & Engine::GetPostTickRoster()
{
	return _post_tick_roster;
}

dBodyID Engine::CreateBody() const
{
	return dBodyCreate(world);
}

CollisionHandle Engine::CreateBox(Vector3 const & dimensions) const
{
	return dCreateBox(space, dimensions.x, dimensions.y, dimensions.z);
}

CollisionHandle Engine::CreateSphere(Scalar radius) const
{
	return dCreateSphere(space, radius);
}

CollisionHandle Engine::CreateRay(Scalar length) const
{
	return dCreateRay(space, length);
}

CollisionHandle Engine::CreateMesh(MeshData data) const
{
	return dCreateTriMesh(space, data, nullptr, nullptr, nullptr);
}

void Engine::DestroyShape(CollisionHandle shape)
{
	dGeomDestroy(shape);
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
	// call objects that want to know that physics is about to be ticked
	_pre_tick_roster.Call();

	if (collisions)
	{
		// Detect / represent all collisions.
		CreateCollisions();
		CreateJoints();
		
		// Tick physics (including acting upon collisions).
		dWorldQuickStep (world, Scalar(delta_time));
		//dWorldStep (world, delta_time);
		
		// Remove this tick's collision data.
		DestroyJoints();
		DestroyCollisions();
	}
	else
	{
		// No collision checking, so just tick physics.
		dWorldQuickStep (world, Scalar(delta_time));
	}
	
	// call objects that want to know that physics has ticked
	_post_tick_roster.Call();
}

form::RayCastResult Engine::CastRay(Ray3 const & ray, Scalar length, Body const * exception)
{
	CRAG_VERIFY_UNIT(ray, .0001f);
	
	// create physics::RayCast object
	RayCast ray_cast(* this, length);
	ray_cast.SetRay(ray);
	
	// don't collide against given exception object
	if (exception != nullptr)
	{
		ray_cast.SetIsCollidable(* exception, false);
	}
	
	// perform collision between ray_cast and all pre-existing objects
	auto handle = ray_cast.GetCollisionHandle();
	dSpaceCollide2(reinterpret_cast<CollisionHandle>(space), handle, nullptr, OnCastRayCollision);

	// return result
	return ray_cast.GetResult();
}

void Engine::Collide(Sphere3 const & sphere, ContactInterface & callback)
{
	// create physics::SphericalBody object
	SphericalBody body(Transformation(sphere.center), nullptr, * this, sphere.radius);
	
	// perform collision between ray_cast and all pre-existing objects
	auto handle = body.GetCollisionHandle();
	dSpaceCollide2(reinterpret_cast<CollisionHandle>(space), handle, & callback, OnSphereCollision);
}

void Engine::ToggleCollisions()
{
	collisions = ! collisions;
}

void Engine::CreateCollisions()
{
	// This basically calls a callback for all the geoms that are quite close.
	dSpaceCollide(space, reinterpret_cast<void *>(this), OnNearCollisionCallback);

	STAT_SET(num_contacts, _contacts.size());
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
void Engine::OnNearCollisionCallback (void * data, CollisionHandle geom1, CollisionHandle geom2)
{
	Engine & engine = ref(reinterpret_cast<Engine *>(data));
	Body & body1 = ref(reinterpret_cast<Body *>(dGeomGetData(geom1)));
	Body & body2 = ref(reinterpret_cast<Body *>(dGeomGetData(geom2)));
	
	if (! body1.IsCollidable(body2) || ! body2.IsCollidable(body1))
	{
		return;
	}
	
	if (body1.OnCollision(body2, engine))
	{
		return;
	}
	
	if (body2.OnCollision(body1, engine))
	{
		return;
	}
	
	engine.OnUnhandledCollision(geom1, geom2);
}

// This is the default handler. It leaves ODE to deal with it. 
void Engine::OnUnhandledCollision(CollisionHandle geom1, CollisionHandle geom2)
{
	// No reason not to keep this nice and high; it's on the stack.
	int constexpr max_contacts_per_collision = 1024;
	ContactGeom contact_geoms [max_contacts_per_collision];
	
	int num_contacts = dCollide (geom1, geom2, max_contacts_per_collision, contact_geoms, sizeof(ContactGeom));
	if (num_contacts == 0)
	{
		return;
	}
	
	// Time to increase max_num_contacts?
	ASSERT (num_contacts * 2 <= max_contacts_per_collision);
	
	(* this)(contact_geoms, contact_geoms + num_contacts);
}

// Called once individual points of contact have been determined.
void Engine::operator() (ContactGeom const * begin, ContactGeom const * end)
{
	auto count = end - begin;
	_contacts.reserve(_contacts.size() + count);
	std::for_each(begin, end, [=] (ContactGeom const & contact_geom)
	{
		// geometry sanity tests
		CRAG_VERIFY_OP(contact_geom.g1, !=, contact_geom.g2);
		CRAG_VERIFY_OP(contact_geom.depth, >=, 0);

		_contact.geom = contact_geom;
		_contacts.push_back(_contact);
		//std::cout << contact.geom.depth << ' ' << contact.geom.normal[0] << ',' << contact.geom.normal[1] << ',' << contact.geom.normal[2] << '\n';

#if defined(DEBUG_CONTACTS)
		Vector3 pos(Convert(contact_geom.pos));
		Vector3 normal(Convert(contact_geom.normal));
		gfx::Debug::AddLine(pos, pos + normal * contact_geom.depth * 100.f);
		//std::cout << pos << ' ' << normal << ' ' << contact_geom.depth << '\n';
#endif
	});
}

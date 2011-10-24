/*
 *  Body.cpp
 *  Crag
 *
 *  Created by John on 10/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Body.h"
#include "Engine.h"


using namespace physics;


Body::Body(Engine & engine, dGeomID init_geom_id, bool movable)
: geom_id(init_geom_id)
{
	// body_id
	if (movable)
	{
		body_id = engine.CreateBody();
		
		dBodySetData(body_id, this);
		dBodySetGravityMode(body_id, false);
		dGeomSetBody(geom_id, body_id);
	}
	else 
	{
		body_id = 0;
	}
	
	dGeomSetData(geom_id, this);
}

Body::~Body()
{
	if (body_id != 0)
	{
		dBodyDestroy(body_id);
	}
	
	Assert(geom_id != 0);
	
	dGeomDestroy(geom_id);
}

dGeomID Body::GetGeomId() const
{
	return geom_id;
}

Scalar Body::GetMass() const
{
	if (body_id == 0)
	{
		return -1;
	}
	
	dMass m;
	dBodyGetMass(body_id, & m);
	Assert(m.mass >= 0);
	
	return m.mass;
}

Vector3 const & Body::GetPosition() const
{
	return * reinterpret_cast<Vector3 const *>(dGeomGetPosition(geom_id));
}

void Body::SetPosition(Vector3 const & position) const
{
	dGeomSetPosition(geom_id, position.x, position.y, position.z);
}

Vector3 Body::GetRelativePointVelocity(Vector3 const & point) const
{
	Vector3 velocity;
	dBodyGetRelPointVel (body_id, point.x, point.y, point.z, velocity.GetAxes());
	return velocity;
}

Vector3 Body::GetVelocity() const
{
	return GetRelativePointVelocity(Vector3::Zero());
}

Matrix33 const & Body::GetRotation() const
{
	return * reinterpret_cast<Matrix33 const *>(dGeomGetRotation(geom_id));
}

void Body::SetRotation(Matrix33 const & matrix)
{
	dGeomSetRotation(geom_id, reinterpret_cast<Scalar const *>(matrix.GetArray()));
}

bool Body::GetIsCollidable() const
{
	unsigned long collide_bits = dGeomGetCollideBits(geom_id);
	return collide_bits != 0;
}

void Body::SetIsCollidable(bool collidable)
{
	unsigned long collide_bits = collidable ? std::numeric_limits<unsigned long>::max() : 0;
	dGeomSetCategoryBits(geom_id, collide_bits);
	dGeomSetCollideBits(geom_id, collide_bits);
}

bool Body::IsMovable() const
{
	return body_id != 0;
}

void Body::SetLinearDamping(Scalar linear_damping)
{
	Assert(body_id != 0);
	dBodySetLinearDamping(body_id, linear_damping);
}

void Body::SetAngularDamping(Scalar angular_damping)
{
	Assert(body_id != 0);
	dBodySetAngularDamping(body_id, angular_damping);
}

void Body::AddRelTorque(Vector3 const & torque)
{
	Assert(body_id != 0);
	dBodyAddRelTorque(body_id, torque.x, torque.y, torque.z);
}

void Body::AddForce(Vector3 const & force)
{
	Assert(body_id != 0);
	dBodyAddForce(body_id, force.x, force.y, force.z);
}

void Body::AddRelForce(Vector3 const & force)
{
	Assert(body_id != 0);
	dBodyAddRelForce(body_id, force.x, force.y, force.z);
}

void Body::AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos)
{
	Assert(body_id != 0);
	dBodyAddForceAtRelPos(body_id, force.x, force.y, force.z, pos.x, pos.y, pos.z);
}

bool Body::OnCollision(Engine & engine, Body const & that_body) const
{
	return false;
}

void Body::OnDeferredCollisionWithBox(Body const & body, IntersectionFunctor & functor) const
{
	Assert(false);
}

void Body::OnDeferredCollisionWithPlanet(Body const & body, IntersectionFunctor & functor) const
{
	Assert(false);
}

void Body::OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctor & functor) const
{
	Assert(false);
}

//
//  Body.cpp
//  crag
//
//  Created by John on 10/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Body.h"
#include "Engine.h"

#include "geom/Matrix33.h"

#include "core/Roster.h"

#include <ode/collision.h>
#include <ode/objects.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::Body member definitions

Body::Body(Transformation const & transformation, Vector3 const * velocity, Engine & engine, dGeomID init_geom_id)
: Location(transformation)
, geom_id(init_geom_id)
, _roster(engine.GetRoster())
{
	// body_id
	if (velocity != nullptr)
	{
		body_id = engine.CreateBody();
		
		dBodySetData(body_id, this);
		dBodySetLinearVel(body_id, velocity->x, velocity->y, velocity->z);
		dBodySetGravityMode(body_id, false);
		dGeomSetBody(geom_id, body_id);
	}
	else 
	{
		body_id = 0;
	}
	
	// geom_id
	dGeomSetData(geom_id, this);

	// set ODE transformation
	SetGeomTransformation(transformation);
	
	// register for physics tick
	_roster.AddCommand(* this, & Body::Tick);
}

Body::~Body()
{
	_roster.RemoveCommand(* this, & Body::Tick);

	if (body_id != 0)
	{
		// destroy all joints associated with the body
		int num_joints = dBodyGetNumJoints (body_id);
		while (num_joints --)
		{
			dJointID joint_id = dBodyGetJoint (body_id, num_joints);
			dJointDestroy(joint_id);
		}
		
		// destroy the body
		dBodyDestroy(body_id);
	}
	
	// destroy the geom
	ASSERT(geom_id != 0);
	dGeomDestroy(geom_id);
}

Body * Body::GetBody()
{
	return this;
}

Body const * Body::GetBody() const
{
	return this;
}

void Body::GetGravitationalForce(Vector3 const & /*pos*/, Vector3 & /*gravity*/) const
{
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
	ASSERT(m.mass >= 0);
	
	return m.mass;
}

void Body::SetTransformation(Transformation const & transformation)
{
	// set ODE value
	SetGeomTransformation(transformation);
	
	// set cached value in Location
	Location::SetTransformation(transformation);
}

Vector3 Body::GetRelativePointVelocity(Vector3 const & point) const
{
	if (body_id == nullptr)
	{
		return Vector3::Zero();
	}

	Vector3 velocity;
	dBodyGetRelPointVel (body_id, point.x, point.y, point.z, velocity.GetAxes());
	return velocity;
}

Vector3 Body::GetVelocity() const
{
	if (body_id == nullptr)
	{
		return Vector3::Zero();
	}

	Vector3 velocity;
	dBodyGetRelPointVel (body_id, 0, 0, 0, velocity.GetAxes());
	return velocity;
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
	ASSERT(body_id != 0);
	dBodySetLinearDamping(body_id, linear_damping);
}

void Body::SetAngularDamping(Scalar angular_damping)
{
	ASSERT(body_id != 0);
	dBodySetAngularDamping(body_id, angular_damping);
}

void Body::AddRelTorque(Vector3 const & torque)
{
	ASSERT(body_id != 0);
	dBodyAddRelTorque(body_id, torque.x, torque.y, torque.z);
}

void Body::AddForce(Vector3 const & force)
{
	ASSERT(body_id != 0);
	dBodyAddForce(body_id, force.x, force.y, force.z);
}

void Body::AddRelForce(Vector3 const & force)
{
	ASSERT(body_id != 0);
	dBodyAddRelForce(body_id, force.x, force.y, force.z);
}

void Body::AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos)
{
	ASSERT(body_id != 0);
	dBodyAddRelForceAtRelPos(body_id, force.x, force.y, force.z, pos.x, pos.y, pos.z);
}

bool Body::OnCollision(Engine &, Body const &) const
{
	return false;
}

void Body::OnDeferredCollisionWithBox(Body const &, IntersectionFunctorRef const &) const
{
	ASSERT(false);
}

void Body::OnDeferredCollisionWithPlanet(Body const &, IntersectionFunctorRef const &) const
{
	ASSERT(false);
}

void Body::OnDeferredCollisionWithSphere(Body const &, IntersectionFunctorRef const &) const
{
	ASSERT(false);
}

void physics::Attach(dJointID joint_id, Body const & body1, Body const & body2)
{
	ASSERT(body1.body_id != nullptr);
	ASSERT(body2.body_id != nullptr);

	dJointAttach(joint_id, body1.body_id, body2.body_id);
	
	Vector3 position1 = body1.GetTranslation();
	Vector3 position2 = body2.GetTranslation();
	Vector3 center = (position1 + position2) * Scalar(.5);
	
	switch (dJointGetType(joint_id))
	{
		case dJointTypeUniversal:
		{
			dJointSetUniversalAnchor (joint_id, center.x, center.y, center.z);
			
			Vector3 between = Normalized(position1 - position2);
			Vector3 axis1 = CrossProduct(between, Vector3(1, 0, 0));
			Vector3 axis2 = CrossProduct(between, axis1);
			
			dJointSetUniversalAxis1 (joint_id, axis1.x, axis1.y, axis1.z);
			dJointSetUniversalAxis2 (joint_id, axis2.x, axis2.y, axis2.z);
			break;
		}
			
		case dJointTypeBall:
		{
			dJointSetBallAnchor (joint_id, position1.x, position1.y, position1.z);
			break;
		}
			
		default:
			// Not yet implemented.
			ASSERT(false);
	}
}

bool physics::IsAttached(Body const & body1, Body const & body2)
{
	if (body1.body_id == nullptr || body2.body_id == nullptr)
	{
		return false;
	}
	
	return dAreConnected(body1.body_id, body2.body_id) != 0;
}

#if defined(VERIFY)
void physics::Body::Verify() const
{
	VerifyObject(GetTranslation());
	VerifyObject(GetVelocity());
	VerifyObject(GetRotation());
	VerifyOp(geom::Length(GetTranslation()), <, 4.0e+8);
//	VerifyOp(geom::Length(GetVelocity()), <, 1000);
}
#endif

Vector3 const & physics::Body::GetGeomTranslation() const
{
	return * reinterpret_cast<Vector3 const *>(dGeomGetPosition(geom_id));
}

void physics::Body::SetGeomTranslation(Vector3 const & translation)
{
	dGeomSetPosition(geom_id, translation.x, translation.y, translation.z);
}

Matrix33 const & physics::Body::GetGeomRotation() const
{
	return * reinterpret_cast<Matrix33 const *>(dGeomGetRotation(geom_id));
}

void physics::Body::SetGeomRotation(Matrix33 const & matrix)
{
	dGeomSetRotation(geom_id, reinterpret_cast<Scalar const *>(matrix.GetArray()));
}

Transformation physics::Body::GetGeomTransformation() const
{
	return Transformation(GetGeomTranslation(), GetGeomRotation());
}

void physics::Body::SetGeomTransformation(Transformation const & transformation)
{
	SetGeomTranslation(transformation.GetTranslation());
	SetGeomRotation(transformation.GetRotation());
}

void physics::Body::Tick()
{
	SetTransformation(GetGeomTransformation());
}

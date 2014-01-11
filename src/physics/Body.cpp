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
#include "RayCast.h"

#include "form/RayCastResult.h"

#include "geom/Matrix33.h"

#include "core/Roster.h"

#include <ode/collision.h>
#include <ode/objects.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::Body member definitions

Body::Body(Transformation const & transformation, Vector3 const * velocity, Engine & engine, CollisionHandle collision_handle)
: Location(transformation)
, _engine(engine)
, _collision_handle(collision_handle)
, _exception(nullptr)
{
	// _body_handle
	if (velocity != nullptr)
	{
		_body_handle = engine.CreateBody();
		
		dBodySetData(_body_handle, this);
		dBodySetLinearVel(_body_handle, velocity->x, velocity->y, velocity->z);
		dBodySetGravityMode(_body_handle, false);
	}
	else 
	{
		_body_handle = nullptr;
	}
	
	// _collision_handle
	if (_collision_handle)
	{
		dGeomSetBody(_collision_handle, _body_handle);
		dGeomSetData(_collision_handle, this);

		// set ODE transformation
		SetGeomTransformation(transformation);
	}
	
	// register for physics tick
	auto & roster = _engine.GetPreTickRoster();
	roster.AddCommand(* this, & Body::Tick);
}

Body::~Body()
{
	auto & roster = _engine.GetPreTickRoster();
	roster.RemoveCommand(* this, & Body::Tick);

	if (_body_handle != 0)
	{
		// destroy all joints associated with the body
		int num_joints = dBodyGetNumJoints (_body_handle);
		while (num_joints --)
		{
			dJointID joint_id = dBodyGetJoint (_body_handle, num_joints);
			dJointDestroy(joint_id);
		}
		
		// destroy the body
		dBodyDestroy(_body_handle);
	}
	
	// destroy the geom
	if (_collision_handle)
	{
		_engine.DestroyShape(_collision_handle);
	}
}

Body * Body::GetBody()
{
	return this;
}

Body const * Body::GetBody() const
{
	return this;
}

bool Body::ObeysGravity() const
{
	return true;
}

void Body::GetGravitationalForce(Vector3 const & /*pos*/, Vector3 & /*gravity*/) const
{
}

BodyHandle Body::GetBodyHandle() const
{
	return _body_handle;
}

CollisionHandle Body::GetCollisionHandle() const
{
	return _collision_handle;
}

void Body::SetDensity(Scalar)
{
	DEBUG_BREAK("Body with no form has no volume, and therefore no density");
}

Scalar Body::GetMass() const
{
	if (_body_handle == 0)
	{
		return -1;
	}
	
	Scalar mass;
	
	dMass m;
	dBodyGetMass(_body_handle, & m);
	mass = m.mass;

	ASSERT(mass >= 0);
	return mass;
}

void Body::SetMass(Mass const &) const
{
	// consider using a method from a derived class
	DEBUG_BREAK("invalid call to Body::SetMess");
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
	if (_body_handle == nullptr)
	{
		return Vector3::Zero();
	}

	Vector3 velocity;

	dBodyGetRelPointVel (_body_handle, point.x, point.y, point.z, velocity.GetAxes());

	return velocity;
}

Vector3 Body::GetVelocity() const
{
	if (_body_handle == nullptr)
	{
		return Vector3::Zero();
	}

	Vector3 velocity;
	dBodyGetRelPointVel (_body_handle, 0, 0, 0, velocity.GetAxes());

	return velocity;
}

bool Body::GetIsCollidable() const
{
	if (! _collision_handle)
	{
		return false;
	}
	
	unsigned long collide_bits = dGeomGetCollideBits(_collision_handle);
	return collide_bits != 0;
}

void Body::SetIsCollidable(bool collidable)
{
	unsigned long collide_bits = collidable ? std::numeric_limits<unsigned long>::max() : 0;
	dGeomSetCategoryBits(_collision_handle, collide_bits);
	dGeomSetCollideBits(_collision_handle, collide_bits);
}

bool Body::IsMovable() const
{
	return _body_handle != 0;
}

void Body::SetLinearDamping(Scalar linear_damping)
{
	ASSERT(_body_handle != 0);
	dBodySetLinearDamping(_body_handle, linear_damping);
}

void Body::SetAngularDamping(Scalar angular_damping)
{
	ASSERT(_body_handle != 0);
	dBodySetAngularDamping(_body_handle, angular_damping);
}

void Body::AddRelTorque(Vector3 const & torque)
{
	ASSERT(_body_handle != 0);
	dBodyAddRelTorque(_body_handle, torque.x, torque.y, torque.z);
}

void Body::AddForce(Vector3 const & force)
{
	ASSERT(_body_handle != 0);
	dBodyAddForce(_body_handle, force.x, force.y, force.z);
}

void Body::AddRelForce(Vector3 const & force)
{
	ASSERT(_body_handle != 0);
	dBodyAddRelForce(_body_handle, force.x, force.y, force.z);
}

void Body::AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos)
{
	ASSERT(_body_handle != 0);
	dBodyAddRelForceAtRelPos(_body_handle, force.x, force.y, force.z, pos.x, pos.y, pos.z);
}

void Body::SetIsCollidable(Body const & body, bool CRAG_DEBUG_PARAM(collidable))
{
	ASSERT(! collidable);	// incomplete implementation
	ASSERT(_exception == nullptr);	// || _exception == & body
	_exception = & body;
}

bool Body::IsCollidable(Body const & body) const
{
	CRAG_VERIFY(body);
	
	return & body != _exception;
}

bool Body::OnCollisionWithSolid(Body &, Sphere3 const &, ContactInterface &)
{
	return false;
}

bool Body::OnCollisionWithRay(Body & that_body)
{
	auto that_collision_handle = that_body.GetCollisionHandle();
	auto this_collision_handle = GetCollisionHandle();

#if defined(NDEBUG)
	constexpr uint16_t max_contacts = 1;
#else
	constexpr uint16_t max_contacts = 2;
#endif
	dContactGeom contacts[max_contacts];
	std::size_t num_contacts = dCollide(this_collision_handle, that_collision_handle, max_contacts, contacts, sizeof(contacts[0]));
	ASSERT(num_contacts <= 1);
	
	if (num_contacts > 0)
	{
		auto & collision_geom = contacts[0];
		ASSERT(collision_geom.g1 == this_collision_handle);
		ASSERT(collision_geom.g2 == that_collision_handle);
		
		auto & ray_cast = static_cast<RayCast &>(that_body);
		ray_cast.SampleResult(form::RayCastResult(physics::Convert(collision_geom.normal), collision_geom.depth, nullptr));
	}
	
	return true;
}

void physics::Attach(dJointID joint_id, Body const & body1, Body const & body2)
{
	ASSERT(body1._body_handle != nullptr);
	ASSERT(body2._body_handle != nullptr);

	dJointAttach(joint_id, body1._body_handle, body2._body_handle);
	
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
	if (body1._body_handle == nullptr || body2._body_handle == nullptr)
	{
		return false;
	}
	
	return dAreConnected(body1._body_handle, body2._body_handle) != 0;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(physics::Body, self)
	CRAG_VERIFY(self.GetTranslation());
	CRAG_VERIFY(self.GetVelocity());
	CRAG_VERIFY(self.GetRotation());
	CRAG_VERIFY_OP(geom::Length(self.GetTranslation()), <, 4.0e+8);
//	CRAG_VERIFY_OP(geom::Length(GetVelocity()), <, 1000);
CRAG_VERIFY_INVARIANTS_DEFINE_END

Vector3 const & physics::Body::GetGeomTranslation() const
{
	return * reinterpret_cast<Vector3 const *>(dGeomGetPosition(_collision_handle));
}

void physics::Body::SetGeomTranslation(Vector3 const & translation)
{
	dGeomSetPosition(_collision_handle, translation.x, translation.y, translation.z);
}

Matrix33 const & physics::Body::GetGeomRotation() const
{
	return * reinterpret_cast<Matrix33 const *>(dGeomGetRotation(_collision_handle));
}

void physics::Body::SetGeomRotation(Matrix33 const & matrix)
{
	dGeomSetRotation(_collision_handle, reinterpret_cast<Scalar const *>(matrix.GetArray()));
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

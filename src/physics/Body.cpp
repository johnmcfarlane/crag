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

using namespace physics;

namespace
{
#if defined(USE_BULLET)
	Vector3 const & Convert(btVector3 const & from)
	{
		auto & to = reinterpret_cast<Vector3 const &>(from);

		ASSERT(&from.x() == &to.x);
		ASSERT(&from.y() == &to.y);
		ASSERT(&from.z() == &to.z);
		
		return to;
	}
	
	btVector3 Convert(Vector3 const & from)
	{
		return btVector3(from.x, from.y, from.z);
	}
	
	Matrix33 Convert(btMatrix3x3 const & from)
	{
		Matrix33 to;
		for (auto row_index = 0; row_index != 3; ++ row_index)
		{
			to.setRow(from.getRow(i));
		}
		return btVector3(from.x, from.y, from.z);
	}
#endif
}

////////////////////////////////////////////////////////////////////////////////
// physics::Body member definitions

Body::Body(Engine & engine, CollisionHandle collision_handle, bool movable)
: _collision_handle(collision_handle)
{
	ASSERT(collision_handle);
	
	if (movable)
	{
#if defined(USE_ODE)
		_body_handle = engine.CreateBody();

		dBodySetData(_body_handle, this);
		dBodySetGravityMode(_body_handle, false);
		dGeomSetBody(_collision_handle, _body_handle);
#endif

#if defined(USE_BULLET)
		_body_handle->setUserPointer(this);
#endif
	}
	else 
	{
		_body_handle = nullptr;
	}
	
#if defined(USE_ODE)
	dGeomSetData(_collision_handle, this);
#endif

#if defined(USE_BULLET)
	_collision_handle->setUserPointer(this);
#endif
}

Body::~Body()
{
	if (_body_handle != 0)
	{
#if defined(USE_ODE)
		// destroy all joints associated with the body
		int num_joints = dBodyGetNumJoints (_body_handle);
		while (num_joints --)
		{
			dJointID joint_id = dBodyGetJoint (_body_handle, num_joints);
			dJointDestroy(joint_id);
		}
		
		// destroy the body
		dBodyDestroy(_body_handle);
#endif

#if defined(USE_BULLET)
		DEBUG_MESSAGE("joints?");
		delete _body_handle;
#endif
	}
	
	// destroy the geom
	ASSERT(_collision_handle != 0);
#if defined(USE_ODE)
	dGeomDestroy(_collision_handle);
#endif
#if defined(USE_BULLET)
	delete _collision_handle;
#endif
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

CollisionHandle Body::GetCollisionHandle() const
{
	return _collision_handle;
}

Scalar Body::GetMass() const
{
	if (_body_handle == 0)
	{
		return -1;
	}
	
	Scalar mass;
	
#if defined(USE_ODE)
	dMass m;
	dBodyGetMass(_body_handle, & m);
	mass = m.mass;
#endif

#if defined(USE_BULLET)
	DEBUG_MESSAGE("not implemented");
#endif
	
	ASSERT(mass >= 0);
	return mass;
}

Vector3 Body::GetPosition() const
{
#if defined(USE_ODE)
	return * reinterpret_cast<Vector3 const *>(dGeomGetPosition(_collision_handle));
#endif

#if defined(USE_BULLET)
	auto & transform = _body_handle->getWorldTransform();
	auto & origin = transform.getOrigin();
	return Convert(origin);
#endif
}

void Body::SetPosition(Vector3 const & position) const
{
#if defined(USE_ODE)
	dGeomSetPosition(_collision_handle, position.x, position.y, position.z);
#endif

#if defined(USE_BULLET)
	auto & transform = _body_handle->getWorldTransform();
	auto & origin = reinterpret_cast<btVector3 const &>(position);
	ASSERT(&origin.x() == &position.x);
	ASSERT(&origin.y() == &position.y);
	ASSERT(&origin.z() == &position.z);
	transform.setOrigin(origin);
#endif
}

Vector3 Body::GetRelativePointVelocity(Vector3 const & point) const
{
	if (_body_handle == nullptr)
	{
		return Vector3::Zero();
	}

	Vector3 velocity;

#if defined(USE_ODE)
	dBodyGetRelPointVel (_body_handle, point.x, point.y, point.z, velocity.GetAxes());
#endif

#if defined(USE_BULLET)
	DEBUG_MESSAGE("not implemented");
#endif

	return velocity;
}

Vector3 Body::GetVelocity() const
{
	if (_body_handle == nullptr)
	{
		return Vector3::Zero();
	}

	Vector3 velocity;
	
#if defined(USE_ODE)
	dBodyGetRelPointVel (_body_handle, 0, 0, 0, velocity.GetAxes());
#endif

#if defined(USE_BULLET)
	_body_handle->getLinearVelocity();
#endif

	return velocity;
}

Matrix33 Body::GetRotation() const
{
#if defined(USE_ODE)
	return * reinterpret_cast<Matrix33 const *>(dGeomGetRotation(_collision_handle));
#endif

#if defined(USE_BULLET)
	DEBUG_MESSAGE("not implemented");
#endif
}

void Body::SetRotation(Matrix33 const & matrix)
{
	dGeomSetRotation(_collision_handle, reinterpret_cast<Scalar const *>(matrix.GetArray()));
}

bool Body::GetIsCollidable() const
{
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

bool Body::OnCollision(Engine & engine, Body const & that_body_handle) const
{
	return false;
}

void Body::OnDeferredCollisionWithBox(Body const & body, IntersectionFunctor & functor) const
{
	ASSERT(false);
}

void Body::OnDeferredCollisionWithPlanet(Body const & body, IntersectionFunctor & functor) const
{
	ASSERT(false);
}

void Body::OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctor & functor) const
{
	ASSERT(false);
}

void physics::Attach(dJointID joint_id, Body const & body1, Body const & body2)
{
	ASSERT(body1._body_handle != nullptr);
	ASSERT(body2._body_handle != nullptr);

	dJointAttach(joint_id, body1._body_handle, body2._body_handle);
	
	Vector3 position1 = body1.GetPosition();
	Vector3 position2 = body2.GetPosition();
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

#if defined(VERIFY)
void physics::Body::Verify() const
{
	VerifyObject(GetPosition());
	VerifyObject(GetVelocity());
	VerifyObject(GetRotation());
	VerifyOp(geom::Length(GetPosition()), <, 4.0e+8);
//	VerifyOp(geom::Length(GetVelocity()), <, 1000);
}
#endif

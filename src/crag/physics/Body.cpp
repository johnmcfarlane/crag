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


physics::Body::Body(sim::Entity & init_entity, dBodyID init_body_id, dGeomID init_geom_id)
: entity(init_entity)
, body_id(init_body_id)
, geom_id(init_geom_id)
{
	dGeomSetBody(geom_id, body_id);
	dGeomSetData(geom_id, this);
}

physics::Body::~Body()
{
}

physics::Vector3 const & physics::Body::GetPosition() const
{
	return * reinterpret_cast<Vector3 const *>(dGeomGetPosition(geom_id));
}

void physics::Body::SetPosition(Vector3 const & position) const
{
	dGeomSetPosition(geom_id, position.x, position.y, position.z);
}

void physics::Body::GetRotation(Matrix4 & rot) const
{
	dReal const * array = dGeomGetRotation(geom_id);
	CopyArray(rot.GetArray(), array, 12);
	rot[3][0] = 0;
	rot[3][1] = 0;
	rot[3][2] = 0;
	rot[3][3] = 1;
}

void physics::Body::SetRotation(Matrix4 const & matrix)
{
	dGeomSetRotation(geom_id, matrix.GetArray());
}

bool physics::Body::IsMovable() const
{
	return body_id != 0;
}

void physics::Body::SetLinearDamping(Scalar linear_damping)
{
	Assert(body_id != 0);
	dBodySetLinearDamping(body_id, linear_damping);
}

void physics::Body::SetAngularDamping(Scalar angular_damping)
{
	Assert(body_id != 0);
	dBodySetAngularDamping(body_id, angular_damping);
}

void physics::Body::AddRelTorque(Vector3 const & torque)
{
	Assert(body_id != 0);
	dBodyAddRelTorque(body_id, torque.x, torque.y, torque.z);
}

void physics::Body::AddRelForce(Vector3 const & force)
{
	Assert(body_id != 0);
	dBodyAddRelForce(body_id, force.x, force.y, force.z);
}

void physics::Body::AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos)
{
	Assert(body_id != 0);
	dBodyAddForceAtRelPos(body_id, force.x, force.y, force.z, pos.x, pos.y, pos.z);
}

bool physics::Body::OnCollisionWithSphericalBody(SphericalBody & that_sphere)
{
	// There is no custom code associated with this type of body to deal with collisions with spheres.
	return false;
}

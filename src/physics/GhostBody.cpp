//
//  GhostBody.cpp
//  crag
//
//  Created by John on 2014-01-09.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "GhostBody.h"

#include <ode/objects.h>

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::GhostBody member definitions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(GhostBody, self)
	CRAG_VERIFY_FALSE(self._collision_handle);
	CRAG_VERIFY_TRUE(self._body_handle);
CRAG_VERIFY_INVARIANTS_DEFINE_END

GhostBody::GhostBody(Transformation const & transformation, Vector3 const & velocity, Engine & engine)
: Body(transformation, & velocity, engine, nullptr)
{
	SetGeomTransformation(transformation);

	CRAG_VERIFY(* this);
}

bool GhostBody::ObeysGravity() const
{
	return false;
}

void GhostBody::SetMass(Mass const & m) const
{
	ASSERT(_body_handle);
	
	dBodySetMass(_body_handle, & m);
}

bool GhostBody::OnCollision(Body &, ContactInterface &)
{
	CRAG_VERIFY(* this);
	
	// benign but wrong
	DEBUG_BREAK("GhostBody collision detected. (Does not have collision geometry.)");
	
	return true;
}

Vector3 const & GhostBody::GetGeomTranslation() const
{
	ASSERT(_body_handle);
	return * reinterpret_cast<Vector3 const *>(dBodyGetPosition(_body_handle));
}

void GhostBody::SetGeomTranslation(Vector3 const & translation)
{
	ASSERT(_body_handle);
	dBodySetPosition(_body_handle, translation.x, translation.y, translation.z);
}

Matrix33 const & GhostBody::GetGeomRotation() const
{
	ASSERT(_body_handle);
	return * reinterpret_cast<Matrix33 const *>(dBodyGetRotation(_body_handle));
}

void GhostBody::SetGeomRotation(Matrix33 const & matrix)
{
	ASSERT(_body_handle);
	dBodySetRotation(_body_handle, reinterpret_cast<Scalar const *>(matrix.GetArray()));
}

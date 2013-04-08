//
//  BoxBody.cpp
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "BoxBody.h"

#include "Engine.h"
#include "SphericalBody.h"


////////////////////////////////////////////////////////////////////////////////
// BoxBody

physics::BoxBody::BoxBody(Engine & engine, bool movable, Vector3 const & dimensions)
: Body(engine, engine.CreateBox(dimensions), movable)
{
}

void physics::BoxBody::SetDimensions(Vector3 const & dimensions) const
{
#if defined(USE_ODE)
	dGeomBoxSetLengths(_collision_handle, dimensions.x, dimensions.y, dimensions.z);
#endif

#if defined(USE_BULLET)
	DEBUG_MESSAGE("not implemented");
#endif
}

physics::Vector3 physics::BoxBody::GetScale() const
{
#if defined(USE_ODE)
	Vector3 dimensions;
	dGeomBoxGetLengths(_collision_handle, dimensions.GetAxes());
	return dimensions;
#endif

#if defined(USE_BULLET)
	DEBUG_MESSAGE("not implemented");
#endif
}

void physics::BoxBody::SetDensity(Scalar density)
{
	ASSERT(_collision_handle != nullptr);
	
#if defined(USE_ODE)
	dMass m;
	Vector3 scale = GetScale();
	dMassSetBox (& m, density, scale.x, scale.y, scale.z);
	dBodySetMass (_body_handle, & m);
#endif

#if defined(USE_BULLET)
	DEBUG_MESSAGE("not implemented");
#endif
}

void physics::BoxBody::OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctor & functor) const
{
	planet.OnDeferredCollisionWithBox(* this, functor);
}

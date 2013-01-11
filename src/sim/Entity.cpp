//
//  Entity.cpp
//  crag
//
//  Created by john on 5/21/09.
//  Copyright 2009-2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Entity.h"

#include "Engine.h"

#include "physics/Body.h"

#include "geom/Transformation.h"


using namespace sim;


//////////////////////////////////////////////////////////////////////
// Entity member definitions


Entity::Entity(super::Init const & init)
: super(init)
, _body(nullptr)
{
}

Entity::~Entity()
{
	delete _body;
}

void Entity::Tick()
{
}

void Entity::GetGravitationalForce(Vector3 const & /*pos*/, Vector3 & /*gravity*/) const
{
}

void Entity::UpdateModels() const
{
}

void Entity::SetBody(Body * body)
{
	ASSERT((_body == nullptr) != (body == nullptr));
	_body = body;
}

Entity::Body * Entity::GetBody()
{
	return _body;
}

Entity::Body const * Entity::GetBody() const
{
	return _body;
}

Transformation Entity::GetTransformation() const
{
	ASSERT(_body != nullptr);
	return Transformation(_body->GetPosition(), _body->GetRotation());
}

#if defined(VERIFY)
void Entity::Verify() const
{
	super::Verify();

	VerifyObjectPtr(_body);
}
#endif

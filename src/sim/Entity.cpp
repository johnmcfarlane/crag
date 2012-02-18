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

#include "Simulation.h"

#include "physics/Body.h"


using namespace sim;


//////////////////////////////////////////////////////////////////////
// Entity member definitions


Entity::Entity()
: _uid(Uid::Create())
, _body(nullptr)
{
}

Entity::~Entity()
{
	delete _body;
}

Uid Entity::GetUid() const
{
	return _uid;
}

void Entity::Tick(Simulation & simulation)
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
	Assert((_body == nullptr) != (body == nullptr));
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

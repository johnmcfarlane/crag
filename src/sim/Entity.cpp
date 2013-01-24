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

#include "gfx/Engine.h"
#include "gfx/object/BranchNode.h"

#include "geom/Transformation.h"


using namespace sim;


//////////////////////////////////////////////////////////////////////
// Entity member definitions

DEFINE_POOL_ALLOCATOR(Entity, 100);

Entity::Entity(super::Init const & init)
: super(init)
, _body(nullptr)
{
}

Entity::~Entity()
{
	delete _body;
	_model.Destroy();
}

void Entity::Tick()
{
}

void Entity::GetGravitationalForce(Vector3 const & /*pos*/, Vector3 & /*gravity*/) const
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

gfx::BranchNodeHandle Entity::GetModel() const
{
	return _model;
}

void Entity::SetModel(gfx::BranchNodeHandle model)
{
	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(false);
	});
	_model = model;
	UpdateModels();
	gfx::Daemon::Call([] (gfx::Engine & engine) {
		engine.OnSetReady(true);
	});
}
	
void Entity::UpdateModels() const
{
	if (_body == nullptr)
	{
		return;
	}
	
	Vector3 position = _body->GetPosition();
	Matrix33 rotation = _body->GetRotation();
	Vector3 scale = _body->GetDimensions() * .5f;
	Transformation transformation(position, rotation, scale);

	_model.Call([transformation] (gfx::BranchNode & node) {
		node.SetTransformation(transformation);
	});
}

#if defined(VERIFY)
void Entity::Verify() const
{
	super::Verify();

	VerifyObjectPtr(_body);
}
#endif

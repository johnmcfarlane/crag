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

#include "Controller.h"
#include "Engine.h"

#include "physics/Location.h"

#include "gfx/Engine.h"
#include "gfx/object/Object.h"

#include "geom/Transformation.h"


using namespace sim;


//////////////////////////////////////////////////////////////////////
// Entity member definitions

DEFINE_POOL_ALLOCATOR(Entity, 100);

Entity::Entity(super::Init const & init)
: super(init)
, _controller(nullptr)
, _location(nullptr)
{
}

Entity::~Entity()
{
	delete _controller;
	delete _location;
	_model.Destroy();
}

// placeholder helps govern the order in which stuff gets called by _tick_roster
void Entity::Tick()
{
	ASSERT(false);
}

void Entity::SetController(Controller * controller)
{
	ASSERT((_controller == nullptr) != (controller == nullptr));
	_controller = controller;
}

Controller * Entity::GetController()
{
	return _controller;
}

void Entity::SetLocation(physics::Location * location)
{
	// TODO: exploit ODE fn, dBodySetMovedCallback(dBodyID, void (*)(dBodyID));
	// TODO: in conjunction with dBodySetData
	ASSERT((_location == nullptr) != (location == nullptr));
	_location = location;
}

physics::Location * Entity::GetLocation()
{
	return _location;
}

physics::Location const * Entity::GetLocation() const
{
	return _location;
}

physics::Body * Entity::GetBody()
{
	if (_location == nullptr)
	{
		return nullptr;
	}

	return _location->GetBody();
}

physics::Body const * Entity::GetBody() const
{
	if (_location == nullptr)
	{
		return nullptr;
	}

	return _location->GetBody();
}

gfx::ObjectHandle Entity::GetModel() const
{
	return _model;
}

void Entity::SetModel(gfx::ObjectHandle model)
{
	_model = model;
}
	
void Entity::UpdateModels() const
{
	if (_location == nullptr || ! _model)
	{
		return;
	}
	
	Vector3 position = _location->GetPosition();
	Matrix33 rotation = _location->GetRotation();
	Vector3 scale = _location->GetScale();
	Transformation transformation(position, rotation, scale);

	_model.Call([transformation] (gfx::Object & node) {
		node.SetLocalTransformation(transformation);
	});
}

#if defined(VERIFY)
void Entity::Verify() const
{
	super::Verify();

	VerifyObjectPtr(_location);

	if (_controller != nullptr)
	{
		VerifyObjectRef(* _controller);
		VerifyEqual(& _controller->GetEntity(), this);
	}
}
#endif

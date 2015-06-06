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

#include "core/RosterObjectDefine.h"

using namespace sim;


//////////////////////////////////////////////////////////////////////
// Entity member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	Entity,
	500,
	Pool::Call<& Entity::UpdateModels>(Engine::GetDrawRoster()))

Entity::Entity(Engine & engine)
: super(engine)
{
	CRAG_VERIFY(* this);
}

Entity::~Entity()
{
	CRAG_VERIFY(* this);

	_model.Release();
}

// placeholder helps govern the order in which stuff gets called by _tick_roster
void Entity::Tick()
{
	ASSERT(false);
}

void Entity::SetController(ControllerPtr controller)
{
	_controller = std::move(controller);

	CRAG_VERIFY(* this);
}

Controller * Entity::GetController()
{
	return _controller.get();
}

Controller const * Entity::GetController() const
{
	return _controller.get();
}

void Entity::SetLocation(LocationPtr location)
{
	// TODO: exploit ODE fn, dBodySetMovedCallback(dBodyID, void (*)(dBodyID));
	// TODO: in conjunction with dBodySetData
	_location = std::move(location);

	CRAG_VERIFY(* this);
}

physics::Location * Entity::GetLocation()
{
	return _location.get();
}

physics::Location const * Entity::GetLocation() const
{
	return _location.get();
}

gfx::ObjectHandle Entity::GetModel() const
{
	return _model;
}

void Entity::SetModel(gfx::ObjectHandle model)
{
	_model = model;
}

void Entity::UpdateModels()
{
	auto location = GetLocation();
	if (location == nullptr || ! _model.IsInitialized())
	{
		return;
	}
	
	Vector3 translation = location->GetTranslation();
	Matrix33 rotation = location->GetRotation();
	Transformation transformation(translation, rotation);

	_model.Call([transformation] (gfx::Object & node) {
		node.SetLocalTransformation(transformation);
	});
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Entity, entity)
	CRAG_ROSTER_OBJECT_VERIFY(entity);
	CRAG_VERIFY(static_cast<Entity::super const &>(entity));

	CRAG_VERIFY(entity._location);
	
	CRAG_VERIFY(entity._controller);
	CRAG_VERIFY(entity._controller);
CRAG_VERIFY_INVARIANTS_DEFINE_END

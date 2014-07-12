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

#include "core/Roster.h"

using namespace sim;


//////////////////////////////////////////////////////////////////////
// Entity member definitions

DEFINE_POOL_ALLOCATOR(Entity);

Entity::Entity(Engine & engine)
: super(engine)
{
	auto & draw_roster = GetEngine().GetDrawRoster();
	draw_roster.AddCommand(* this, & Entity::UpdateModels);
}

Entity::~Entity()
{
	auto & draw_roster = GetEngine().GetDrawRoster();
	draw_roster.RemoveCommand(* this, & Entity::UpdateModels);

	_model.Release();
}

// placeholder helps govern the order in which stuff gets called by _tick_roster
void Entity::Tick()
{
	ASSERT(false);
}

void Entity::SetController(ControllerPtr const & controller)
{
	_controller = controller;

	CRAG_VERIFY(* this);
}

Entity::ControllerPtr & Entity::GetController()
{
	return _controller;
}

Entity::ControllerPtr const & Entity::GetController() const
{
	return _controller;
}

void Entity::SetLocation(LocationPtr const & location)
{
	// TODO: exploit ODE fn, dBodySetMovedCallback(dBodyID, void (*)(dBodyID));
	// TODO: in conjunction with dBodySetData
	_location = location;

	CRAG_VERIFY(* this);
}

Entity::LocationPtr & Entity::GetLocation()
{
	return _location;
}

Entity::LocationPtr const & Entity::GetLocation() const
{
	return _location;
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
	
	Vector3 translation = _location->GetTranslation();
	Matrix33 rotation = _location->GetRotation();
	Transformation transformation(translation, rotation);

	_model.Call([transformation] (gfx::Object & node) {
		node.SetLocalTransformation(transformation);
	});
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Entity, entity)
	CRAG_VERIFY(static_cast<Entity::super const &>(entity));

	CRAG_VERIFY(entity._location);
	
	CRAG_VERIFY(entity._controller);
	CRAG_VERIFY(entity._controller);
CRAG_VERIFY_INVARIANTS_DEFINE_END

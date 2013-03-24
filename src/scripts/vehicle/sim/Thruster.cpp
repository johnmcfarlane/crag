//
//  sim/Thruster.cpp
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Thruster.h"

#include "sim/axes.h"
#include "sim/Entity.h"
#include "sim/Engine.h"

#include "../gfx/Thruster.h"

#include "physics/Body.h"

#include "gfx/Engine.h"

#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::Thruster member definitions

Thruster::Thruster(Entity & entity, Ray3 const & ray)
	: _entity(entity)
	, _ray(ray)
	, _thrust_factor(0)
{
	// calculate local transformation
	auto thrust_scale = Length(ray.direction);
	Transformation local_transformation(ray.position, axes::Rotation(ray.direction / thrust_scale), thrust_scale);

	// create model
	_model = gfx::ThrusterHandle::CreateHandle(local_transformation);
	
	// roster
	auto& tick_roster = entity.GetEngine().GetTickRoster();
	tick_roster.AddCommand(* this, & Thruster::Tick);

	VerifyObject(* this);
}

Thruster::~Thruster()
{
	VerifyObject(* this);

	// destroy model
	_model.Destroy();

	// roster
	auto& tick_roster = GetEntity().GetEngine().GetTickRoster();
	tick_roster.RemoveCommand(* this, & Thruster::Tick);
}

#if defined(VERIFY)
void Thruster::Verify() const
{
	VerifyRef(_entity);
	VerifyTrue(_model);
	VerifyOp(_thrust_factor, >=, 0);
}
#endif

void Thruster::SetParentModel(gfx::ObjectHandle parent_model)
{
	auto uid = _model.GetUid();
	auto parent_uid = parent_model.GetUid();
	gfx::Daemon::Call([uid, parent_uid] (gfx::Engine & engine) {
		engine.OnSetParent(uid, parent_uid);
	});
}

Entity & Thruster::GetEntity()
{
	return _entity;
}

float Thruster::GetThrustFactor() const
{
	VerifyObject(* this);

	return _thrust_factor;
}

void Thruster::SetThrustFactor(float thrust_factor)
{
	VerifyObject(* this);

	_thrust_factor = thrust_factor;

	VerifyObject(* this);
}

void Thruster::UpdateModel() const
{
	VerifyObject(* this);

	float thrust_factor = _thrust_factor;
	if (thrust_factor <= 0)
	{
		return;
	}

	_model.Call([thrust_factor] (gfx::Thruster & thruster) 
	{
		thruster.Update(thrust_factor);
	});
}

void Thruster::Tick()
{
	VerifyObject(* this);

	if (_thrust_factor <= 0)
	{
		return;
	}

	auto body = GetEntity().GetBody();
	if (body != nullptr)
	{
		body->AddRelForceAtRelPos(_ray.direction * _thrust_factor, _ray.position);
	}
}

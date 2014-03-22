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

#include "sim/Entity.h"
#include "sim/Engine.h"

#include "../gfx/Thruster.h"

#include "physics/Body.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"

#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::Thruster member definitions

Thruster::Thruster(Entity & entity, Ray3 const & ray, bool graphical, Scalar thrust_factor)
	: _entity(entity)
	, _ray(ray)
	, _thrust_factor(thrust_factor)
{
	// create model
	if (graphical)
	{
		// calculate local transformation
		auto thrust_max = Length(ray.direction);
		if (thrust_max)
		{
			Transformation local_transformation(ray.position, gfx::Rotation(ray.direction / thrust_max));

			_model = gfx::ThrusterHandle::CreateHandle(local_transformation, thrust_max);
		}
	}

	// roster
	auto & tick_roster = entity.GetEngine().GetTickRoster();

	// register tick
	tick_roster.AddOrdering(& Thruster::Tick, & Entity::Tick);
	tick_roster.AddCommand(* this, & Thruster::Tick);
	
	// register draw
	auto & draw_roster = entity.GetEngine().GetDrawRoster();
	draw_roster.AddCommand(* this, & Thruster::UpdateModel);

	CRAG_VERIFY(* this);
}

Thruster::~Thruster()
{
	CRAG_VERIFY(* this);

	// destroy model
	_model.Destroy();

	// unregister draw
	auto & draw_roster = GetEntity().GetEngine().GetDrawRoster();
	draw_roster.RemoveCommand(* this, & Thruster::UpdateModel);

	// unregister tick
	auto & tick_roster = GetEntity().GetEngine().GetTickRoster();
	tick_roster.RemoveCommand(* this, & Thruster::Tick);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Thruster, self)
	CRAG_VERIFY(self._entity);
	CRAG_VERIFY_OP(self._thrust_factor, >=, 0);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Thruster::SetParentModel(gfx::ObjectHandle parent_model)
{
	if (! _model)
	{
		return;
	}

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

Ray3 const & Thruster::GetRay() const
{
	return _ray;
}

void Thruster::SetRay(Ray3 const & ray)
{
	_ray = ray;

	if (_model)
	{
		// create model
		_model.Call([ray] (gfx::Object & model)
		{
			// calculate new local light transformation
			auto thrust_max = Length(ray.direction);
			Transformation local_transformation(ray.position, gfx::Rotation(ray.direction / thrust_max));
			model.SetLocalTransformation(local_transformation);
		});
	}
}

float Thruster::GetThrustFactor() const
{
	CRAG_VERIFY(* this);

	return _thrust_factor;
}

void Thruster::SetThrustFactor(float thrust_factor)
{
	CRAG_VERIFY(* this);

	_thrust_factor = thrust_factor;

	CRAG_VERIFY(* this);
}

void Thruster::Tick()
{
	CRAG_VERIFY(* this);

	if (_thrust_factor <= 0)
	{
		return;
	}

	auto location = GetEntity().GetLocation();
	auto & body = core::StaticCast<physics::Body>(ref(location));
	body.AddRelForceAtRelPos(_ray.direction * _thrust_factor, _ray.position);
}

void Thruster::UpdateModel() const
{
	CRAG_VERIFY(* this);

	float thrust_factor = _thrust_factor;
	if (thrust_factor <= 0)
	{
		return;
	}

	if (_model)
	{
		_model.Call([thrust_factor] (gfx::Thruster & thruster) 
		{
			thruster.Update(thrust_factor);
		});
	}
}

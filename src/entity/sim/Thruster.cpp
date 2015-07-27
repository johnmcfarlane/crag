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
#include <sim/Model.h>

#include "../gfx/Thruster.h"

#include "physics/Body.h"
#include <physics/Engine.h>

#include "gfx/Engine.h"

#include <geom/utils.h>

#include "core/RosterObjectDefine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::Thruster member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	Thruster,
	2000,
	Pool::Call<& Thruster::Tick>(physics::Engine::GetPreTickRoster()),
	Pool::Call<& Thruster::UpdateModel>(Engine::GetDrawRoster()))

Thruster::Thruster(Entity & entity, Ray3 const & ray, bool graphical, Scalar thrust_factor)
	: Receiver(thrust_factor)
	, _entity(entity)
	, _ray(ray)
{
	// create model
	if (graphical)
	{
		// calculate local transformation
		auto thrust_max = Magnitude(ray.direction);
		if (thrust_max)
		{
			Transformation local_transformation(ray.position, geom::Rotation(ray.direction / thrust_max));

			_model = gfx::ThrusterHandle::Create(local_transformation, thrust_max);
		}

		SetParentModel(* entity.GetModel());
	}

	CRAG_VERIFY(* this);
}

Thruster::~Thruster() noexcept
{
	CRAG_VERIFY(* this);

	// release model
	_model.Release();
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Thruster, self)
	CRAG_VERIFY(self._entity);
	CRAG_VERIFY_OP(self.GetSignal(), >=, 0);
	CRAG_VERIFY_OP(self.GetSignal(), <=, 1.f);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Thruster::SetParentModel(Model const & parent_model)
{
	CRAG_VERIFY_TRUE(_model.IsInitialized());

	auto parent_handle = parent_model.GetHandle();
	auto model_handle = _model;
	gfx::Daemon::Call([model_handle, parent_handle] (gfx::Engine & engine) {
		engine.OnSetParent(model_handle, parent_handle);
	});
}

Entity & Thruster::GetEntity()
{
	return _entity;
}

Entity const & Thruster::GetEntity() const
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

	if (_model.IsInitialized())
	{
		// create model
		_model.Call([ray] (gfx::Object & model)
		{
			// calculate new local light transformation
			auto thrust_max = Magnitude(ray.direction);
			Transformation local_transformation(ray.position, geom::Rotation(ray.direction / thrust_max));
			model.SetLocalTransformation(local_transformation);
		});
	}
}

void Thruster::Tick()
{
	CRAG_VERIFY(* this);

	if (GetSignal() <= 0)
	{
		return;
	}

	auto & location = * GetEntity().GetLocation();
	auto & body = core::StaticCast<physics::Body>(location);
	auto & ray = _ray;
	body.AddRelForceAtRelPos(ray.direction * static_cast<Scalar>(GetSignal()), ray.position);
}

void Thruster::UpdateModel()
{
	CRAG_VERIFY(* this);

	float thrust_factor = GetSignal();
	if (thrust_factor <= 0)
	{
		return;
	}

	auto & model = _model;
	if (model.IsInitialized())
	{
		model.Call([thrust_factor] (gfx::Thruster & thruster)
		{
			thruster.Update(thrust_factor);
		});
	}
}

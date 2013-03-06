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

#include "../gfx/Thruster.h"

#include "physics/Body.h"

#include "gfx/Engine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::Thruster member definitions

Thruster::Thruster(Ray3 const & ray, gfx::ThrusterHandle const & model)
	: _ray(ray)
	, _model(model)
	, _thrust_factor(0)
{
	VerifyObject(* this);
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

void Thruster::ApplyThrust(physics::Body & body) const
{
	VerifyObject(* this);

	if (_thrust_factor <= 0)
	{
		return;
	}

	body.AddRelForceAtRelPos(_ray.direction * _thrust_factor, _ray.position);
}

#if defined(VERIFY)
void Thruster::Verify() const
{
	VerifyTrue(_model);
	VerifyOp(_thrust_factor, >=, 0);
}
#endif

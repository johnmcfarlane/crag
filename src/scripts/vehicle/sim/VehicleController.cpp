//
//  sim/VehicleController.cpp
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "VehicleController.h"

#include "../gfx/Thruster.h"

#include "sim/axes.h"
#include "sim/Entity.h"

#include "gfx/Engine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::VehicleController member functions

// seems to be required by MetaClass::InitObject
VehicleController::VehicleController(Entity & entity)
: _super(entity)
{
}

ThrusterPtr VehicleController::AddThruster(Ray3 const & ray)
{
	// calculate local transformation
	auto thrust_scale = Length(ray.direction);
	Transformation local_transformation(ray.position, axes::Rotation(ray.direction / thrust_scale), thrust_scale);

	// create actual thruster
	auto model = gfx::ThrusterHandle::CreateHandle(local_transformation);
	auto thruster_uid = model.GetUid();
	
	// thruster's parent is vehicle
	auto & entity = GetEntity();
	auto parent_model = entity.GetModel();
	gfx::Daemon::Call([thruster_uid, parent_model] (gfx::Engine & engine) {
		engine.OnSetParent(thruster_uid, parent_model.GetUid());
	});

	// create/add thruster to vector
	_thrusters.push_back(ThrusterPtr(new Thruster(ray, model)));

	return _thrusters.back();
}

void VehicleController::Tick()
{
	auto& entity = GetEntity();
	physics::Body * body = entity.GetBody();
	if (body == nullptr)
	{
		// vehicle is 'broken' because body was invalidated and destroyed.
		return;
	}

	TickThrusters();
	
	for (ThrusterVector::iterator i = _thrusters.begin(), end = _thrusters.end(); i != end; ++ i)
	{
		auto & thruster = * * i;
		thruster.ApplyThrust(* body);
		thruster.UpdateModel();
	}
}

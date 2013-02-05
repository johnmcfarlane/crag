//
//  VehicleController.cpp
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "VehicleController.h"

#include "axes.h"
#include "Entity.h"
#include "EntityFunctions.h"
#include "Engine.h"

#include "physics/Body.h"

#include "gfx/object/BranchNode.h"
#include "gfx/object/Thruster.h"
#include "gfx/Engine.h"

#include "core/app.h"


using namespace sim;


////////////////////////////////////////////////////////////////////////////////
// sim::VehicleController member functions

DEFINE_POOL_ALLOCATOR(VehicleController, 1);

// seems to be required by MetaClass::InitObject
VehicleController::VehicleController(Entity & entity)
: _super(entity)
{
}

void VehicleController::AddThruster(Thruster const & thruster)
{
	ASSERT(! thruster.model);
	
	// Create sim-side thruster object and get ref to it for writing.
	_thrusters.push_back(thruster);
	Thruster & _thruster = _thrusters.back();
	
	// create gfx-side branch node
	Scalar thrust_scale = Length(_thruster.direction);
	Transformation transformation(_thruster.position, axes::Rotation(_thruster.direction / thrust_scale), thrust_scale);
	auto branch_node = gfx::BranchNodeHandle::CreateHandle(transformation);
	
	// branch node's parent is vehicle's branch node
	sim::Entity const & entity = GetEntity();
	gfx::ObjectHandle const & parent_model = entity.GetModel();
	gfx::Daemon::Call([branch_node, parent_model] (gfx::Engine & engine) {
		engine.OnSetParent(branch_node.GetUid(), parent_model.GetUid());
	});
	
	// create actual thruster graphics
	_thruster.model.Create();
	auto thruster_uid = _thruster.model.GetUid();
	
	// its parent is the branch node
	gfx::Daemon::Call([thruster_uid, branch_node] (gfx::Engine & engine) {
		engine.OnSetParent(thruster_uid, branch_node.GetUid());
	});
	
	// initialize thrust factor
	_thruster.thrust_factor = 0;
}

void VehicleController::UpdateModels() const
{
	// For each thruster,
	for (ThrusterVector::const_iterator i = _thrusters.begin(), end = _thrusters.end(); i != end; ++ i)
	{
		auto thrust_factor = i->thrust_factor;
		if (thrust_factor != 0)
		{
			i->model.Call([thrust_factor] (gfx::Thruster & thruster) {
				thruster.Update(thrust_factor);
			});
		}
	}
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
	
	for (ThrusterVector::iterator i = _thrusters.begin(), end = _thrusters.end(); i != end; ++ i)
	{
		Thruster & Thruster = * i;
		TickThruster(Thruster, * body);
	}

	UpdateModels();
}

void VehicleController::TickThruster(Thruster & Thruster, physics::Body & body)
{
	if (app::IsKeyDown(Thruster.key))
	{
		Thruster.thrust_factor = 1;
		body.AddRelForceAtRelPos(Thruster.direction, Thruster.position);
	}
	else
	{
		Thruster.thrust_factor = 0;
	}
}

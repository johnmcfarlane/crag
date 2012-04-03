//
//  Vehicle.cpp
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Vehicle.h"

#include "axes.h"
#include "EntityFunctions.h"
#include "Engine.h"

#include "physics/Body.h"

#include "gfx/object/BranchNode.h"
#include "gfx/object/Thruster.h"
#include "gfx/Renderer.h"

#include "core/app.h"


using namespace sim;


////////////////////////////////////////////////////////////////////////////////
// sim::Vehicle member functions

// seems to be required by MetaClass::InitObject
Vehicle::Vehicle()
{
}

void Vehicle::AddThruster(Thruster const & thruster)
{
	ASSERT(! thruster.model);
	
	// Create sim-side thruster object and get ref to it for writing.
	_thrusters.push_back(thruster);
	Thruster & _thruster = _thrusters.back();
	
	// create gfx-side branch node
	gfx::BranchNodeHandle branch_node;
	Scalar thrust_scale = Length(_thruster.direction);
	Transformation transformation(_thruster.position, axes::Rotation(_thruster.direction / thrust_scale), thrust_scale);
	branch_node.Create(transformation);
	
	// branch node's parent is vehicle's branch node
	gfx::ObjectHandle const & parent_model = GetModel();
	gfx::Daemon::Call(& gfx::Renderer::OnSetParent, branch_node.GetUid(), parent_model.GetUid());
	
	// create actual thruster graphics
	_thruster.model.Create();
	
	// its parent is the branch node
	gfx::Daemon::Call(& gfx::Renderer::OnSetParent, _thruster.model.GetUid(), branch_node.GetUid());
	
	// initialize thrust factor
	_thruster.thrust_factor = 0;
}

gfx::Color4f Vehicle::GetColor() const
{
	return gfx::Color4f::White();
}

void Vehicle::UpdateModels() const
{
	super::UpdateModels();
	
	// For each thruster,
	for (ThrusterVector::const_iterator i = _thrusters.begin(), end = _thrusters.end(); i != end; ++ i)
	{
		Thruster const & thruster = * i;		
		if (thruster.thrust_factor != 0)
		{
			thruster.model.Call<float>(& gfx::Thruster::Update, thruster.thrust_factor);
		}
	}
}

void Vehicle::Tick(sim::Engine & simulation_engine)
{
	physics::Body * body = GetBody();
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
}

void Vehicle::TickThruster(Thruster & Thruster, Body & body)
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

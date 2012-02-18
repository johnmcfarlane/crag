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
#include "Simulation.h"

#include "physics/Body.h"

#include "gfx/object/BranchNode.h"
#include "gfx/object/Thruster.h"
#include "gfx/Renderer.h"
#include "gfx/Renderer.inl"

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
	Assert(thruster.gfx_uid == gfx::Uid::null);
	
	// Create sim-side thruster object and get ref to it for writing.
	_thrusters.push_back(thruster);
	Thruster & _thruster = _thrusters.back();
	
	// Create graphical representation of thruster.
	gfx::Object * gfx_thruster = new gfx::Thruster;
	gfx::Uid parent_uid = GetGfxUid();
	gfx::Uid transformation_uid = AddModelWithTransform(* gfx_thruster, parent_uid);
	gfx::Uid thruster_uid = gfx_thruster->GetUid();
	
	// Position graphical representation.
	Scalar thrust_scale = Length(_thruster.direction);
	gfx::BranchNode::UpdateParams params = 
	{
		Transformation(_thruster.position, axes::Rotation(_thruster.direction / thrust_scale), thrust_scale)
	};
	
	gfx::Daemon::Call(transformation_uid, params, & gfx::Renderer::OnUpdateObject<gfx::BranchNode>);
	
	// Initialize the rest of the sim-side data for the new thruster.
	_thruster.gfx_uid = thruster_uid;
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
		
		// send the amount of thrust to the graphical representation.
		gfx::Thruster::UpdateParams params = 
		{
			thruster.thrust_factor
		};
		
		if (params.thrust_factor != 0)
		{
			gfx::Daemon::Call(thruster.gfx_uid, params, & gfx::Renderer::OnUpdateObject<gfx::Thruster>);
		}
	}
}

void Vehicle::Tick(Simulation & simulation) override
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

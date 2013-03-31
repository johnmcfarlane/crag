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

#include "sim/Entity.h"
#include "sim/Engine.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::VehicleController member functions

VehicleController::VehicleController(Entity & entity)
: _super(entity)
{
	VerifyObject(* this);
}

VehicleController::~VehicleController()
{
	VerifyObject(* this);

	while (! _thrusters.empty())
	{
		PopThruster();
	}
}

#if defined(VERIFY)
void VehicleController::Verify() const
{
	for (auto & thruster : _thrusters)
	{
		VerifyRef(* thruster);
		VerifyEqual(& thruster->GetEntity(), & GetEntity());
	}
}
#endif

void VehicleController::AddThruster(Thruster * thruster)
{
	VerifyObject(* this);

	// model
	auto & entity = GetEntity();
	auto parent_model = entity.GetModel();
	thruster->SetParentModel(parent_model);

	// add to vector
	_thrusters.push_back(thruster);

	VerifyObject(* this);
}

void VehicleController::PopThruster()
{
	VerifyObject(* this);
	ASSERT(! _thrusters.empty());

	// get last thruster
	auto thruster = _thrusters.back();

	// remove it from vector
	_thrusters.pop_back();

	// delete it
	delete thruster;

	VerifyObject(* this);
}

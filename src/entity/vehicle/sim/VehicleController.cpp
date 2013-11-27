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
	CRAG_VERIFY(* this);
}

VehicleController::~VehicleController()
{
	CRAG_VERIFY(* this);

	while (! _thrusters.empty())
	{
		PopThruster();
	}
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(VehicleController, self)
	for (auto & thruster : self._thrusters)
	{
		CRAG_VERIFY(* thruster);
		CRAG_VERIFY_EQUAL(& thruster->GetEntity(), & self.GetEntity());
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END

void VehicleController::AddThruster(Thruster * thruster)
{
	CRAG_VERIFY(* this);

	// model
	auto & entity = GetEntity();
	auto parent_model = entity.GetModel();
	thruster->SetParentModel(parent_model);

	// add to vector
	_thrusters.push_back(thruster);

	CRAG_VERIFY(* this);
}

void VehicleController::PopThruster()
{
	CRAG_VERIFY(* this);
	ASSERT(! _thrusters.empty());

	// get last thruster
	auto thruster = _thrusters.back();

	// remove it from vector
	_thrusters.pop_back();

	// delete it
	delete thruster;

	CRAG_VERIFY(* this);
}

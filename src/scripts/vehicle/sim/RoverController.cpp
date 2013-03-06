//
//  RoverController.cpp
//  crag
//
//  Created by John McFarlane on 2013/03/04.
//	Copyright 2013 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RoverController.h"

#include "core/app.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// sim::VehicleController member functions

DEFINE_DEFAULT_ALLOCATOR(RoverController);

RoverController::RoverController(Entity & entity)
: VehicleController(entity)
{
}

void RoverController::AddThruster(Ray3 const & ray, SDL_Scancode key)
{
	auto thruster = VehicleController::AddThruster(ray);
	_inputs.insert(std::make_pair(key, thruster));	// TODO: _inputs.emplace(key, thruster)
}

void RoverController::TickThrusters()
{
	for (auto& input : _inputs)
	{
		auto key = input.first;
		bool is_key_down = app::IsKeyDown(key);

		auto& thruster = *input.second;
		thruster.SetThrustFactor(is_key_down);
	}
}

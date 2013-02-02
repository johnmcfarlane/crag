//
//  Vehicle.h
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Entity.h"


namespace gfx { DECLARE_CLASS_HANDLE(Thruster); }	// gfx::ThrusterHandle

namespace physics
{
	class Body;
}

namespace sim
{
	// An entity with its own locomotion.
	class Vehicle : public Entity
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef Entity super;
	public:
		typedef gfx::ThrusterHandle ThrusterHandle;
		
		struct Thruster
		{
			Vector3 position;	// position of Thruster relative to vehicle
			Vector3 direction;	// direction of thrust relative to vehicle rotation
			SDL_Scancode key;
			ThrusterHandle model;
			float thrust_factor;
		};
		
		typedef std::vector<Thruster> ThrusterVector;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		DECLARE_ALLOCATOR(Vehicle);

		Vehicle(Init const & init);
		
		// Add a Thruster
		void AddThruster(Thruster const & thruster);
		
	private:
		// Called when sending messages to the renderer thread.
		void UpdateModels() const override;
		
		// Called whenever the simulation ticks.
		void Tick() override;

		// Add force from an individual Thruster to the vehicle's body.
		static void TickThruster(Thruster & Thruster, physics::Body & body);

		////////////////////////////////////////////////////////////////////////////////
		// data
		ThrusterVector _thrusters;
	};
}

//
//  Vehicle.h
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Ball.h"


namespace sim
{
	// An entity with its own locomotion.
	class Vehicle : public Ball
	{
		DECLARE_SCRIPT_CLASS(Vehicle, Ball);
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		struct Rocket
		{
			Vector3 position;	// position of rocket relative to vehicle
			Vector3 direction;	// direction of thrust relative to vehicle rotation
			SDL_Scancode key;
		};
		
		typedef std::vector<Rocket> RocketVector;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		// Add a rocket
		void AddRocket(Rocket const & rocket);

		// Called to create a bare object.
		static void Create(Vehicle & vehicle, PyObject & args);
		
	private:
		// Called from the simulation thread to initialize the object.
		bool Init(Simulation & simulation, PyObject & args) override;
		
		// Called when sending messages to the renderer thread.
		void UpdateModels() const override;
		
		// Called whenever the simulation ticks.
		void Tick(Simulation & simulation) override;

		// Add force from an individual rocket to the vehicle's body.
		static void ApplyForce(Rocket const & rocket, Body & body);

		////////////////////////////////////////////////////////////////////////////////
		// data
		RocketVector _rockets;
	};
}

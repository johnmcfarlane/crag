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


namespace gfx 
{
	class Thruster;
}


namespace sim
{
	// An entity with its own locomotion.
	class Vehicle : public Ball
	{
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef Ball super;
	public:
		typedef smp::Handle<gfx::Thruster> ThrusterHandle;
		
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
		
		Vehicle();
		
		// Add a Thruster
		void AddThruster(Thruster const & thruster);
		
	private:
		// Called from Ball when initializing the graphics object.
		gfx::Color4f GetColor() const override;
		
		// Called when sending messages to the renderer thread.
		void UpdateModels() const override;
		
		// Called whenever the simulation ticks.
		void Tick(Simulation & simulation) override;

		// Add force from an individual Thruster to the vehicle's body.
		static void TickThruster(Thruster & Thruster, Body & body);

		////////////////////////////////////////////////////////////////////////////////
		// data
		ThrusterVector _thrusters;
	};
}

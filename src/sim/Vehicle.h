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
		DECLARE_SCRIPT_CLASS(Vehicle, Ball);
		
		////////////////////////////////////////////////////////////////////////////////
		// types

	public:
		struct Thruster;
		typedef std::vector<Thruster> ThrusterVector;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		// Add a Thruster
		void AddThruster(Thruster const & thruster);

		// Called to create a bare object.
		static void Create(Vehicle & vehicle, PyObject & args);
		
	private:
		// Called from the simulation thread to initialize the object.
		bool Init(Simulation & simulation, PyObject & args) override;
		
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

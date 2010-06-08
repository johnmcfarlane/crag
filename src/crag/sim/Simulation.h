/*
 *  Simulation.h
 *  Crag
 *
 *  Created by John on 10/19/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Physics.h"

#include "gfx/Debug.h"
#include "gfx/Renderer.h"
#include "gfx/Scene.h"

#include "core/Vector3.h"

#include "core/App.h"


namespace form
{
	class Manager;
}

namespace sim
{
	class Observer;
	
	// TODO: New class, Crag, which deals with app stuff. Simulation to deal with less critical real-time issues.

	// Top-level class, deals with running the simulation and presenting it to screen. 
	class Simulation
	{
	public:
		Simulation();
		~Simulation();

		void Run();
		
	private:
		void InitUniverse();
		
		void Tick();
		bool PollMesh();
		
		void Render();
		void PrintStats() const;
		void Capture();
		
		// Returns true until the program should ent.
		bool HandleEvents();
		bool OnKeyPress(app::KeyCode key_code);
		
		app::TimeType GetTime(bool update = false);

		// Attributes
		gfx::Renderer renderer;
		gfx::Scene scene;
		Physics physics;
		Observer * observer;
		form::Manager * formation_manager;

		bool paused;
		bool capture;
		int capture_frame;
		
		app::TimeType running_poll_seconds;
		
		// Periodically recalculat fps
		double fps;
		int frame_count;
		app::TimeType frame_count_reset_time;
		
		app::TimeType cached_time;
	};
}

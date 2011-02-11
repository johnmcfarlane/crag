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

#include "Universe.h"

#include "gfx/Renderer.h"
#include "gfx/Scene.h"

#include "geom/Vector3.h"

#include "sys/App.h"

#include "smp/Singleton.h"


namespace form
{
	class Manager;
}


namespace sim
{

	// Top-level class, deals with running the simulation and presenting it to screen. 
	class Simulation : public smp::Singleton<Simulation>
	{
	public:
		Simulation(bool init_enable_vsync);
		~Simulation();
	
	private:
		void Init();
	public:
		
		Universe const & GetUniverse() const;
		
		gfx::Scene & GetScene();
		gfx::Scene const & GetScene() const;
		
		void AddEntity(Entity & entity);
		void RemoveEntity(Entity & entity);
		
		void SetCameraPos(Vector3 const & pos, Matrix4 const & rot);
		
		void Run();
	private:
		void Tick();
		
		void Render();
		void PrintStats() const;
		void Capture();
		
		// Returns true until the program should ent.
		bool HandleEvents();
		bool OnKeyPress(sys::KeyCode key_code);

		// Attributes
		Universe universe;

		form::Manager * formation_manager;

		bool enable_vsync;
		bool paused;
		bool capture;
		int capture_frame;
		
		gfx::Scene scene;
		gfx::Renderer renderer;
		
		sys::TimeType start_time;
	};
	
}

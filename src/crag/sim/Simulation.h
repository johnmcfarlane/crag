/*
 *  Simulation.h
 *  Crag
 *
 *  Created by John on 10/19/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "Physics.h"

#include "gfx/DebugGraphics.h"
#include "gfx/Renderer.h"
#include "gfx/Scene.h"

#include "core/Vector3.h"

#include "app/App.h"


namespace form
{
	class Manager;
}

namespace sim
{
	class Observer;

	class Simulation
	{
	public:
		Simulation();
		~Simulation();

		void Run();
		
	private:
		void InitUniverse();
		
		void Tick();
		
		//void AdjustTargetNumNodes();
		//float CalculateTargetNumNodes(float current_num_nodes, float tick_period_min, float tick_period_max);
		
		void Render();
		void PrintStats() const;
		void Capture();
		
		// Returns true until the program should ent.
		bool HandleEvents();
		bool OnKeyPress(app::KeyCode key_code);

		// Attributes
		gfx::Renderer renderer;
		gfx::Scene scene;
		Physics physics;
		Observer * observer;
		form::Manager * formation_manager;

		bool paused;
		bool capture;
		int capture_frame;
		
		int max_consecutive_frame_lag;
		int consecutive_frame_lag;
		
		// Periodically recalculat fps
		float fps;
		int frame_count;
		app::TimeType frame_count_reset_time;
	};
}

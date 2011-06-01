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

#include "defs.h"

#include "smp/Actor.h"

#include "core/ConfigEntry.h"
#include "core/Singleton.h"

#include "sys/App.h"


namespace gfx 
{
	class Renderer;
	class Scene;
}

namespace physics 
{
	class Engine;
}


namespace sim
{
	
	class Entity;
	class Simulation;
	class Universe;
	
	
	// TODO: Tidy this horrible lot up.
	class Observer;
	struct AddObserverMessage
	{
		Vector3 center;
	};
	

	class Planet;
	struct AddPlanetMessage
	{
		Vector3 center;
		Scalar radius;
		int random_seed;
		int num_craters;
	};
	
	
	class Star;
	struct AddStarMessage
	{
		Scalar orbital_radius;
		Scalar orbital_year;
	};
	
	
	struct RemoveEntityMessage
	{
		Entity & entity;
	};
	
	
	class Simulation : public smp::Actor<Simulation>
	{
		OBJECT_SINGLETON(Simulation);
	public:
		CONFIG_DECLARE_MEMBER (target_frame_seconds, sys::TimeType);

		Simulation(bool init_enable_vsync = true);
		~Simulation();
		
		// Singleton
		static Simulation & Ref() { return ref(singleton); }
	
		// Message passing
		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message) 
		{ 
			Simulation & destination = Ref(); 
			smp::Actor<Simulation>::SendMessage(destination, message); 
		}
		
		template <typename MESSAGE, typename RESULT>
		static void SendMessage(MESSAGE const & message, RESULT & result) 
		{ 
			Simulation & destination = Ref(); 
			smp::Actor<Simulation>::SendMessage(destination, message, result); 
		}
		
		void OnMessage(smp::TerminateMessage const & message);
		void OnMessage(AddObserverMessage const & message, Observer * & reply);
		void OnMessage(AddPlanetMessage const & message, Planet * & reply);
		void OnMessage(AddStarMessage const & message, Star * & reply);
		void OnMessage(RemoveEntityMessage const & message);
		void OnMessage(SetCameraMessage const & message);
		
	private:
		void Init();
		
	public:
		sys::TimeType GetTime() const;
		
		Universe & GetUniverse();

		physics::Engine & GetPhysicsEngine();		
		
		gfx::Scene & GetScene();
		
	private:
		void AddEntity(Entity & entity);
		void RemoveEntity(Entity & entity);
		
	public:		
		bool HandleEvent(sys::Event const & event);

	private:
		void Run();
		void Tick();
		
		void Render();
		void PrintStats() const;
		void Capture();
		
		// Returns true until the program should ent.
		bool OnKeyPress(sys::KeyCode key_code);

		// Attributes
		bool enable_vsync;
		bool paused;
		bool capture;
		int capture_frame;
		
		Universe * universe;
		physics::Engine * physics_engine;

		gfx::Scene * scene;
		gfx::Renderer * renderer;
		
		sys::TimeType start_time;
		
		static Simulation * singleton;
	};
	
}

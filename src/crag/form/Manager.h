/*
 *  form/Manager.h
 *  Crag
 *
 *  Created by john on 5/23/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "MeshBufferObject.h"

#include "sim/defs.h"
#include "core/Singleton.h"
#include "core/Thread.h"

#include <set>


namespace sim
{
	class Observer;
}

namespace gfx
{
	class Pov;
}


namespace form {

	class Formation;
	class SceneThread;
	
	typedef std::set<Formation *> FormationSet;
	
	
	class Manager : public core::Singleton<Manager>
	{
	public:
		Manager(sim::Observer & init_observer);
		~Manager();
		
#if VERIFY
		void Verify() const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(Manager);
		
		void AddFormation(Formation * formation);
		void RemoveFormation(Formation * formation);
		FormationSet const & GetFormations() const;
		
		void AdjustNumNodes(float frame_delta, float target_frame_delta);
		void ToggleSuspended();
		
		void Launch();
		void Tick();
		bool PollMesh();

		// Called by the Renderer.
		void Render(gfx::Pov const & pov, bool color) const;

	private:
		static bool InitMultithreading();
		
		FormationSet formation_set;
		sim::Observer & observer;

		bool regenerating;
		SceneThread * scene_thread;

		// The graphicy things.
		// TODO: Are two objects currently necessary?
		form::MeshBufferObject buffer_objects[2];
		form::MeshBufferObject * volatile front_buffer_object;
		form::MeshBufferObject * volatile back_buffer_object;
		sim::Vector3 front_buffer_origin;
	};	

}

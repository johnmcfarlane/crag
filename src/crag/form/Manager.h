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

#include "form/scene/MeshBufferObject.h"

#include "sim/defs.h"

#include "core/Singleton.h"

#include "sys/App.h"
#include "sys/Thread.h"

#if defined(FORM_VERTEX_TEXTURE)
#include "glpp/Texture.h"
#endif

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

	class FormationFunctor;
	class Formation;
	class SceneThread;
	
	typedef std::set<Formation *> FormationSet;
	
	
	// TODO: Consider making this not a singleton.
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
		
		void AdjustNumNodes(app::TimeType frame_delta, app::TimeType target_frame_delta);
		void ToggleSuspended();
		void ToggleFlatShaded();
		
		void Launch();
		void Tick();
		void ForEachFormation(FormationFunctor & f) const;
		bool PollMesh();

		// Called by the Renderer.
		void Render(gfx::Pov const & pov, bool color) const;

	private:
		void RenderFormations(gfx::Pov const & pov, bool color) const;
		void DebugStats() const;
		
#if defined(FORM_VERTEX_TEXTURE)
		bool InitTexture();
#endif
		
		FormationSet formation_set;
		sim::Observer & observer;

		bool suspended;
		bool regenerating;
		SceneThread * scene_thread;

#if defined(FORM_VERTEX_TEXTURE)
		gl::TextureRgba8 texture;
#endif
		
		// The graphicy things.
		form::MeshBufferObject buffer_objects[2];
		form::MeshBufferObject * volatile front_buffer_object;
		form::MeshBufferObject * volatile back_buffer_object;
	};	

}

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

#include "core/double_buffer.h"
#include "core/Singleton.h"

#include "smp/Thread.h"

#include "sys/App.h"

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
		
		void SampleFrameRatio(sys::TimeType frame_delta, sys::TimeType target_frame_delta);
		void ToggleSceneThread();
		void ToggleMeshGeneration();
		void ToggleDynamicOrigin();
		void ToggleFlatShaded();
		
		void Launch();
		void Tick();
		void ForEachFormation(FormationFunctor & f) const;
		bool PollMesh();
		void ResetRegulator();

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

		bool enable_mesh_generation;
		SceneThread * scene_thread;

#if defined(FORM_VERTEX_TEXTURE)
		gl::TextureRgba8 texture;
#endif
		
		core::double_buffer<form::MeshBufferObject> mesh_buffers;
	};

}

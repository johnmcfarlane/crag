/*
 *  form/FormationManager.h
 *  Crag
 *
 *  Created by john on 5/23/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "form/scene/MeshBufferObject.h"
#include "form/scene/Regulator.h"
#include "form/scene/Scene.h"

#include "sim/defs.h"

#include "core/double_buffer.h"
#include "core/Singleton.h"

#include "smp/Thread.h"
#include "smp/Semaphore.h"
#include "smp/Singleton.h"

#include "sys/App.h"

#include <set>


namespace gfx
{
	class Pov;
}


namespace form {

	class FormationFunctor;
	class Formation;
	
	typedef std::set<Formation *> FormationSet;
	
	
	class Keeper
	{
	};
	
	
	class FormationManager : public smp::Singleton<FormationManager>
	{
	public:
		FormationManager();
		~FormationManager();
		
		void Run();
		void Exit();
		
		static void AddFormation(Formation & formation);
		static void RemoveFormation(Formation & formation);		
		void ForEachFormation(FormationFunctor & f) const;
		
		static void SampleFrameRatio(sys::TimeType frame_delta, sys::TimeType target_frame_delta);
		static void ResetRegulator();
		
		static void ToggleSuspended();
		static void ToggleMeshGeneration();
		static void ToggleDynamicOrigin();
		static void ToggleFlatShaded();
		
		static void SetCameraPos(sim::Ray3 const & camera_pos);
		static void PollMesh();

		// Called by the Renderer.
		static void Render(gfx::Pov const & pov, bool color);
	private:
		static void RenderFormations(gfx::Pov const & pov, bool color);
		static void DebugStats();

		void Tick();
		void TickActiveScene();
		void GenerateMesh();

		void AdjustNumQuaterna();
		void BeginReset();
		void EndReset();
		
		Scene & GetActiveScene();
		Scene const & GetActiveScene() const;
		
		Scene & GetVisibleScene();
		Scene const & GetVisibleScene() const;
		
		bool IsOriginOk() const;
		bool IsGrowing() const;
		bool IsResetting() const;
		
		
		// attributes
		FormationSet formation_set;

		bool quit_flag;
		bool suspend_flag;
		bool enable_mesh_generation;
		
		volatile bool back_mesh_buffer_ready;
		
		typedef core::double_buffer<Mesh *> MeshDoubleBuffer;
		MeshDoubleBuffer meshes;
		
		sys::TimeType mesh_generation_time;
		
		typedef core::double_buffer<MeshBufferObject> MboDoubleBuffer;
		MboDoubleBuffer mesh_buffers;

		Regulator regulator;
		sim::Ray3 _camera_pos;

		// The front scene is always the one being displayed and is usually the 'active' scene.
		// During an origin reset, the back scene is the active one.
		// It's the active scene which is reshaped for LODding purposes (churned).
		core::double_buffer<Scene> scenes;
		bool is_in_reset_mode;	// the scene is being regenerated following an origin reset
		
		smp::Semaphore suspend_semaphore;		
	};

}

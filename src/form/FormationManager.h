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

#include "form/scene/Mesh.h"
#include "form/scene/Regulator.h"
#include "form/scene/Scene.h"

#include "sim/defs.h"

#include "core/double_buffer.h"
#include "core/Singleton.h"

#include "smp/Daemon.h"
#include "smp/scheduler.h"
#include "smp/Semaphore.h"

#include "sys/App.h"


namespace gfx
{
	class Pov;
	class FormationSet;
}


namespace form 
{	
	
	////////////////////////////////////////////////////////////////////////////////
	// forward declarations
	
	class FormationFunctor;
	class Formation;
	class MeshBufferObject;
	class Scene;
	
	typedef std::set<Formation *> FormationSet;
	
	
	// form::Daemon type
	class FormationManager;
	typedef smp::Daemon<FormationManager> Daemon;
	
	
	////////////////////////////////////////////////////////////////////////////////
	// FormationManager class
	
	// The top-most formation management class.
	class FormationManager
	{
		OBJECT_SINGLETON(FormationManager);
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		struct TreeQueryFunctor
		{
			virtual ~TreeQueryFunctor() { }
			virtual void operator () (Scene const & scene) = 0;
			
		};
		
		typedef core::ring_buffer<smp::scheduler::Job, true> BatchedFunctorBuffer;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		FormationManager();
		~FormationManager();
		
#if defined(VERIFY)
		// Verification
		void Verify() const;
#endif
		
		// message interface
		void OnQuit();
		void OnAddFormation(Formation * const & formation);
		void OnRemoveFormation(Formation * const & formation);
		void OnSetMesh(Mesh * const & mesh);
		void OnSetCamera(sim::Transformation const & transformation);
		
		void OnRegulatorReset();
		void OnRegulatorSetNumQuaterna(int const & num_quaterne);
		void OnRegulatorSetFrame(float const & fitness);
		
		void Run(Daemon::MessageQueue & message_queue);
	public:
		
		// intersection support
		void LockTree();
		void UnlockTree();
		Scene const & OnTreeQuery() const;
		
		void ToggleSuspended();
		void ToggleMeshGeneration();
		void ToggleDynamicOrigin();
		void ToggleFlatShaded();
		
	private:
		void SetCamera(sim::Transformation const & transformation);
		
		void Tick();
		void TickActiveScene();
		void GenerateMesh();
		Mesh * PopMesh();
		
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
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef core::double_buffer<Scene> SceneDoubleBuffer;
		gfx::FormationSet & _model;
		
		// attributes
		FormationSet formation_set;
		
		bool quit_flag;
		bool suspend_flag;
		bool enable_mesh_generation;
		bool flat_shaded_flag;
		
		Mesh::list_type _meshes;
		
		sys::TimeType mesh_generation_time;
		
		Regulator _regulator;
		sim::Ray3 _camera_pos;
		
		// The front scene is always the one being displayed and is usually the 'active' scene.
		// During an origin reset, the back scene is the active one.
		// It's the active scene which is reshaped for LODding purposes (churned).
		SceneDoubleBuffer scenes;
		bool is_in_reset_mode;	// the scene is being regenerated following an origin reset
		
		static Daemon * singleton;
	};
	
}

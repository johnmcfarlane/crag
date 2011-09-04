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

#include "form/scene/Regulator.h"
#include "form/scene/Scene.h"

#include "sim/defs.h"

#include "core/double_buffer.h"
#include "core/Singleton.h"

#include "smp/Daemon.h"
#include "smp/scheduler.h"
#include "smp/Semaphore.h"

#include "sys/App.h"

#include <set>


namespace gfx
{
	class Pov;
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
	
	
	////////////////////////////////////////////////////////////////////////////////
	// FormationManager message classes
	
	struct AddFormationMessage
	{
		Formation & formation;
	};
	
	struct RemoveFormationMessage
	{
		Formation & formation;
	};
	
	
	////////////////////////////////////////////////////////////////////////////////
	// FormationManager class
	
	// The top-most formation management class.
	class FormationManager
	{
		OBJECT_SINGLETON(FormationManager);
		
	public:
		// types
		typedef smp::Daemon<FormationManager> Daemon;
		
		struct IntersectionFunctor
		{
			IntersectionFunctor(sim::Sphere3 const & sphere, Formation const & formation) : _sphere(sphere), _formation(formation) { }
			virtual ~IntersectionFunctor() { }
			virtual void operator()(sim::Vector3 const & pos, sim::Vector3 const & normal, sim::Scalar depth) = 0;
			
			sim::Sphere3 const _sphere;
			Formation const & _formation;
		};

		typedef core::ring_buffer<smp::scheduler::Job, true> BatchedFunctorBuffer;

		// functions
		FormationManager();
		~FormationManager();
		
#if defined(VERIFY)
		// Verification
		void Verify() const;
#endif
		
		void OnMessage(smp::TerminateMessage const & message);
		void OnMessage(AddFormationMessage const & message);
		void OnMessage(RemoveFormationMessage const & message);
		void OnMessage(sim::SetCameraMessage const & message);

		void Run(Daemon::MessageQueue & message_queue);
	private:		
		void AddFormation(Formation & formation);
		void RemoveFormation(Formation & formation);
	public:
		
		// Individual intersection tests.
		void ForEachIntersection(IntersectionFunctor & functor) const;

		// batched formation tests (node tree is locked for the duration of the batch)
		void ForEachTreeQuery(BatchedFunctorBuffer & functor_buffer) const;

		// Intersection test called from within ForEachTreeQuery.
		// TODO: Yes, this interface is a little messed up.
		void ForEachIntersectionLocked(IntersectionFunctor & functor) const;
		
		void SampleFrameRatio(sys::TimeType frame_delta, sys::TimeType target_frame_delta);
		void ResetRegulator();
		
		void ToggleSuspended();
		void ToggleMeshGeneration();
		void ToggleDynamicOrigin();
		void ToggleFlatShaded();
		
	private:
		void SetCameraPos(sim::CameraProjection const & projection);
	public:
		bool PollMesh(MeshBufferObject & back_mesh);

		// Called by the Renderer.
		void Render();
	private:

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
		
		// types
		typedef core::double_buffer<Mesh *> MeshDoubleBuffer;
		typedef core::double_buffer<Scene> SceneDoubleBuffer;
		
		// attributes
		FormationSet formation_set;

		bool quit_flag;
		bool suspend_flag;
		bool enable_mesh_generation;
		
		volatile bool back_mesh_buffer_ready;
		
		MeshDoubleBuffer meshes;
		
		sys::TimeType mesh_generation_time;
		int rendered_num_quaternia;
		
		Regulator regulator;
		sim::Ray3 _camera_pos;

		// The front scene is always the one being displayed and is usually the 'active' scene.
		// During an origin reset, the back scene is the active one.
		// It's the active scene which is reshaped for LODding purposes (churned).
		SceneDoubleBuffer scenes;
		bool is_in_reset_mode;	// the scene is being regenerated following an origin reset
		
		// TODO: This is pre-messages. Perhaps there's a better way now?
		smp::Semaphore suspend_semaphore;		
		
		static Daemon * singleton;
	};

}

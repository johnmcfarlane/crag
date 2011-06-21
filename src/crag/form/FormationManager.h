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

#include "smp/Actor.h"
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
	class FormationManager : public smp::Actor<FormationManager>
	{
		OBJECT_SINGLETON(FormationManager);
	public:
		FormationManager();
		~FormationManager();
		
		// Verification
#if defined(VERIFY)
		void Verify() const;
#endif
		
		// Singleton
		static FormationManager & Ref() { return ref(singleton); }
		
		// Message passing
		template <typename MESSAGE>
		static void SendMessage(MESSAGE const & message, bool blocking) 
		{ 
			FormationManager & formation_manager = Ref();
			smp::Actor<FormationManager>::SendMessage(formation_manager, message, blocking); 
		}
		
		void OnMessage(smp::TerminateMessage const & message);
		void OnMessage(AddFormationMessage const & message);
		void OnMessage(RemoveFormationMessage const & message);
		void OnMessage(sim::SetCameraMessage const & message);

	private:
		void Run();
		void Exit();
		
		void AddFormation(Formation & formation);
		void RemoveFormation(Formation & formation);
		
	public:
		struct IntersectionFunctor
		{
			virtual ~IntersectionFunctor() { }
			virtual void operator()(sim::Vector3 const & pos, sim::Vector3 const & normal, sim::Scalar depth) = 0;
		};
		void ForEachIntersection(sim::Sphere3 const & sphere, Formation const & formation, IntersectionFunctor & functor) const;
		
		void SampleFrameRatio(sys::TimeType frame_delta, sys::TimeType target_frame_delta);
		void ResetRegulator();
		
		void ToggleSuspended();
		void ToggleMeshGeneration();
		void ToggleDynamicOrigin();
		void ToggleFlatShaded();
		
	private:
		void SetCameraPos(sim::CameraProjection const & projection);
	public:
		void PollMesh();

		// Called by the Renderer.
		void Render(gfx::Pov const & pov, bool color);
	private:
		void RenderFormations(gfx::Pov const & pov, bool color);
		void DebugStats();

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
		typedef core::double_buffer<MeshBufferObject> MboDoubleBuffer;
		typedef core::double_buffer<Scene> SceneDoubleBuffer;
		
		// attributes
		FormationSet formation_set;

		bool quit_flag;
		bool suspend_flag;
		bool enable_mesh_generation;
		
		volatile bool back_mesh_buffer_ready;
		
		MeshDoubleBuffer meshes;
		
		sys::TimeType mesh_generation_time;
		
		MboDoubleBuffer mbo_buffers;

		Regulator regulator;
		sim::Ray3 _camera_pos;

		// The front scene is always the one being displayed and is usually the 'active' scene.
		// During an origin reset, the back scene is the active one.
		// It's the active scene which is reshaped for LODding purposes (churned).
		SceneDoubleBuffer scenes;
		bool is_in_reset_mode;	// the scene is being regenerated following an origin reset
		
		smp::Semaphore suspend_semaphore;		
		
		static FormationManager * singleton;
	};

}

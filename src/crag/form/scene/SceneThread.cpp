/*
 *  form/SceneThread.cpp
 *  Crag
 *
 *  Created by John on 10/12/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "form/scene/SceneThread.h"
#include "form/Manager.h"

#include "sim/Observer.h"
#include "sim/axes.h"

#include "physics/Body.h"

#include "gfx/Debug.h"

#include "core/ConfigEntry.h"
#include "core/profile.h"


namespace 
{
	CONFIG_DEFINE (max_observer_position_length, sim::Scalar, 2500);
	
	PROFILE_DEFINE (scene_tick_period, .1);
	PROFILE_DEFINE (scene_tick_per_quaterna, .1);
	PROFILE_DEFINE (mesh_generation_period, .1);	
	PROFILE_DEFINE (mesh_generation_per_quaterna, .1);
}


form::SceneThread::SceneThread(FormationSet const & _formations, sim::Observer const & _observer, bool _threaded)
: formations(_formations)
, observer(_observer)
, threaded(_threaded)
, reset_origin_flag(false)
, suspend_flag(false)
, quit_flag(false)
, mesh(form::NodeBuffer::max_num_verts, static_cast<int>(form::NodeBuffer::max_num_verts * 1.25f))
, mesh_updated(false)
, mesh_generation_time(0)
, suspend_semaphore(1)
{
}

#if VERIFY
void form::SceneThread::Verify() const
{
	VerifyObject(scenes [0]);
	VerifyObject(scenes [1]);
}
#endif

#if DUMP
DUMP_OPERATOR_DEFINITION(form, SceneThread)
{
	lhs << lhs.NewLine() << "scene:";
	
	DumpStream indented (lhs);
	indented << rhs.scene;
	
	return lhs;
}
#endif

void form::SceneThread::Launch()
{
	Assert(IsMainThread());	
	Assert(! thread.IsLaunched());

	if (threaded) 
	{
		thread.Launch(* this);
	}
}

void form::SceneThread::ToggleSuspended()
{
	suspend_flag = ! suspend_flag;
	
	if (threaded)
	{
		if (suspend_flag)
		{
			suspend_semaphore.Decrement();
		}
		else
		{
			suspend_semaphore.Increment();
		}
	}
}

void form::SceneThread::Quit()
{
	Assert(IsMainThread());
	
	quit_flag = true;

	suspend_semaphore.Increment();
	thread.Join();
}

void form::SceneThread::SampleFrameRatio(float ratio)
{
	Assert(IsMainThread());
	
	regulator.SampleFrameRatio(ratio);
}

// Should be called whenever there is the possability of a change in 
// performance of the system, such as startup or on certain system messages.
void form::SceneThread::ResetRegulator()
{
	regulator = Regulator();
}

void form::SceneThread::Tick()
{
	Assert(IsMainThread());

	if (! threaded && ! suspend_flag) 
	{
		TickThread();
	}
	
	if (gfx::Debug::GetVerbosity() > 0.2)
	{
		sim::Vector3 const & observer_pos = observer.GetPosition();
		double observer_position_length = Length(observer_pos - GetActiveScene().GetOrigin());
		gfx::Debug::out << "oor:" << max_observer_position_length - observer_position_length << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .15)
	{
		gfx::Debug::out << "scene_t:" << PROFILE_RESULT(scene_tick_period) << '\n';
		gfx::Debug::out << "meshg_t:" << PROFILE_RESULT(mesh_generation_period) << '\n';
		
		std::ios_base::fmtflags flags = gfx::Debug::out.flags(std::ios::fixed);
		std::streamsize previous_precision = gfx::Debug::out.precision(10);
		gfx::Debug::out << "scene_t/q(us):" << 1000000.f * PROFILE_RESULT(scene_tick_per_quaterna) << '\n';
		gfx::Debug::out << "meshg_t/q(ms):" << 1000.f * PROFILE_RESULT(mesh_generation_per_quaterna) << '\n';
		gfx::Debug::out.flags(flags);
		gfx::Debug::out.precision(previous_precision);
	}
	
	if (gfx::Debug::GetVerbosity() > .72) 
	{
		gfx::Debug::out << "nodes:" << GetVisibleScene().GetNumNodesUsed() << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .17) 
	{
		gfx::Debug::out << "visible_q:" << GetVisibleScene().GetNumQuaternaUsed() << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .37) 
	{
		gfx::Debug::out << " active_q:" << GetActiveScene().GetNumQuaternaUsed() << '\n';
		gfx::Debug::out << " target_q:" << GetActiveScene().GetNumQuaternaUsedTarget() << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .2)
	{
		if (IsResetting())
		{
			gfx::Debug::out << "resetting...\n";
		}
	}
	
	if (suspend_flag && gfx::Debug::GetVerbosity() > .05)
	{
		gfx::Debug::out << "disabled: scene thread\n";
	}
}

void form::SceneThread::ForEachFormation(FormationFunctor & f) const
{
	GetVisibleScene().ForEachFormation(f);
}

bool form::SceneThread::PollMesh(MeshBufferObject & mbo)
{
	Assert(IsMainThread());
	
	if (! mesh_updated || ! mesh_semaphore.TryDecrement())
	{
		return false;
	}
		
	bool polled;
	
	if (mesh_updated)
	{
		mbo.Set(mesh);

		mesh_updated = false;
		polled = true;
	}
	else
	{
		polled = false;
	}
	
	mesh_semaphore.Increment();
	
	return polled;
}

void form::SceneThread::ResetOrigin()
{
	Assert(IsMainThread());

	reset_origin_flag = true;
}

void form::SceneThread::ToggleFlatShaded()
{
	bool & flat_shaded = mesh.GetProperties().flat_shaded;
	flat_shaded = ! flat_shaded;
}

// Returns false if a new origin is needed.
bool form::SceneThread::IsOriginOk() const
{
	if (reset_origin_flag) 
	{
		// We're on it already!
		return true;
	}
	
	if (IsGrowing() || IsResetting())
	{
		// Still making the last one
		return true;
	}
	
	// The real test: Is the observer not far enough away from the 
	// current origin that visible inaccuracies might become apparent?
	sim::Vector3 const & observer_pos = observer.GetPosition();
	double observer_position_length = Length(observer_pos - GetVisibleScene().GetOrigin());
	return observer_position_length < max_observer_position_length;
}

bool form::SceneThread::IsResetting() const
{
	return is_in_reset_mode;
}

bool form::SceneThread::IsGrowing() const
{
	Scene const & active_scene = GetActiveScene();
	int num_used = active_scene.GetNumQuaternaUsed();
	int num_used_target = active_scene.GetNumQuaternaUsedTarget();
	return num_used < num_used_target;
}

form::Scene & form::SceneThread::GetActiveScene()
{
	return IsResetting() ? scenes.back() : scenes.front();
}

form::Scene const & form::SceneThread::GetActiveScene() const
{
	return IsResetting() ? scenes.back() : scenes.front();
}

form::Scene & form::SceneThread::GetVisibleScene()
{
	return scenes.front();
}

form::Scene const & form::SceneThread::GetVisibleScene() const
{
	return scenes.front();
}

// The main loop of the scene thread. Hopefully it's pretty self-explanatory.
void form::SceneThread::Run()
{
	while (true) 
	{
		suspend_semaphore.Decrement();
		if (quit_flag)
		{
			break;
		}
		
		TickThread();

		suspend_semaphore.Increment();
		if (quit_flag)
		{
			break;
		}
	}
}

// The tick function of the scene thread. 
// This functions gets called repeatedly either in the scene thread - if there is on -
// or in the main thread as part of the main render/simulation iteration.
void form::SceneThread::TickThread()
{
	if (reset_origin_flag) 
	{
		BeginReset();
	}
	else 
	{
		AdjustNumQuaterna();
	}

	TickActiveScene();
	
	if (IsResetting())
	{
		if (! IsGrowing())
		{
			EndReset();
		}
	}

	GenerateMesh();
}

void form::SceneThread::TickActiveScene()
{
	Scene & active_scene = GetActiveScene();
	sim::Ray3 camera_ray = observer.GetCameraRay();
	active_scene.SetCameraRay(camera_ray);
	
	{
		PROFILE_TIMER_BEGIN(t);
		
		active_scene.Tick(formations);
		
		PROFILE_SAMPLE(scene_tick_per_quaterna, PROFILE_TIMER_READ(t) / active_scene.GetNumQuaternaUsed());
		PROFILE_SAMPLE(scene_tick_period, PROFILE_TIMER_READ(t));
	}
}

void form::SceneThread::BeginReset()
{
	Assert(! IsResetting());
	is_in_reset_mode = true;
	
	Scene & active_scene = GetActiveScene();
	
	sim::Vector3 const & observer_pos = observer.GetPosition();
	active_scene.SetOrigin(observer_pos);
	
	int current_num_quaterna = GetVisibleScene().GetNumQuaternaUsed();
	active_scene.SetNumQuaternaUsedTarget(current_num_quaterna);
	
	reset_origin_flag = false;
}

void form::SceneThread::EndReset()
{
	is_in_reset_mode = false;
	scenes.flip();
}

void form::SceneThread::AdjustNumQuaterna()
{
	if (IsResetting())
	{
		return;
	}
	
	// Get the regulator input.
	Scene & active_scene = GetActiveScene();	
	int current_num_quaterna = active_scene.GetNumQuaternaUsed();
	
	// Calculate the regulator output.
	int target_num_quaterna = regulator.GetAdjustedLoad(current_num_quaterna);
	target_num_quaterna = Min(target_num_quaterna, int(NodeBuffer::max_num_quaterna));
	
	// Apply the regulator output.
	active_scene.SetNumQuaternaUsedTarget(target_num_quaterna);
}

void form::SceneThread::GenerateMesh()
{
	Assert(IsSceneThread());
	
/*	if (IsGrowing())
	{
		return;
	}*/
	
	if (IsResetting())
	{
		return;
	}
	
	if (mesh_updated)
	{
		// No point making multiple meshes if main thread isn't polling them fast enough.
		return;
	}
	
	if (! mesh_semaphore.TryDecrement())
	{
		// Don't wait around if current mesh is being read by main thread;
		// This thread should be locked as little as possible.
		return;
	}
	
	Scene & visible_scene = GetVisibleScene();
	visible_scene.GenerateMesh(mesh);
	mesh_semaphore.Increment();

	mesh_updated = true;
	sys::TimeType t = sys::GetTime();
	sys::TimeType last_mesh_generation_period = t - mesh_generation_time;
	regulator.SampleMeshGenerationPeriod(last_mesh_generation_period);
	mesh_generation_time = t;
	
	PROFILE_SAMPLE(mesh_generation_per_quaterna, last_mesh_generation_period / GetActiveScene().GetNumQuaternaUsed());
	PROFILE_SAMPLE(mesh_generation_period, last_mesh_generation_period);
}

bool form::SceneThread::IsMainThread() const
{
	return ! threaded || ! thread.IsCurrent();
}

bool form::SceneThread::IsSceneThread() const
{
	return ! threaded || thread.IsCurrent();
}

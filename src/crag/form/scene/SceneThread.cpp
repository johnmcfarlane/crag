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


namespace 
{
	CONFIG_DEFINE (max_observer_position_length, sim::Scalar, 2500);
	CONFIG_DEFINE (max_mesh_generation_period, app::TimeType, .2f);
	CONFIG_DEFINE (post_reset_freeze_period, app::TimeType, 1.25f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient, float, 0.01f);
	CONFIG_DEFINE (max_mesh_generation_reaction_coefficient, float, 0.9975f);	// Multiply node count by this number when mesh generation is too slow.
	CONFIG_DEFINE (dynamic_origin, bool, true);
}


form::SceneThread::SceneThread(FormationSet const & _formations, sim::Observer const & _observer, bool _threaded)
: num_nodes(100)
, frame_ratio(-1)
, scene_tick_period(0)
, formations(_formations)
, observer(_observer)
, threaded(_threaded)
, reset_origin_flag(false)
, suspend_flag(false)
, quit_flag(false)
, origin_reset_time(0)
, mesh(form::NodeBuffer::max_num_verts, static_cast<int>(form::NodeBuffer::max_num_verts * 1.25f))
, mesh_updated(false)
, mesh_generation_time(0)
, mesh_generation_period(max_mesh_generation_period)
, thread(NULL)
{
}

#if VERIFY
void form::SceneThread::Verify() const
{
	VerifyObject(scene);
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
	
	if (threaded) {
		Assert(thread == nullptr);
		thread = new core::Thread(ThreadFunc, this);
	}
}

void form::SceneThread::Quit()
{
	Assert(IsMainThread());
	
	quit_flag = true;

	if (thread)
	{
		thread->Join();
	}
}

void form::SceneThread::SetFrameRatio(float ratio)
{
	Assert(IsMainThread());
	
	float clipped_frame_ratio = Max(ratio, std::numeric_limits<float>::min());
	
	if (frame_ratio < 0)
	{
		frame_ratio = clipped_frame_ratio;
	}
	else 
	{
		frame_ratio = Max(frame_ratio, clipped_frame_ratio);
	}
}

int form::SceneThread::GetNumQuaternaUsed() const
{
	return scene.GetNumQuaternaUsed();
}

int form::SceneThread::GetNumQuaternaUsedTarget() const
{
	return scene.GetNumQuaternaUsedTarget();
}

void form::SceneThread::Tick()
{
	Assert(IsMainThread());

	if (! threaded) 
	{
		ThreadTick();
	}
	
	if (gfx::Debug::GetVerbosity() > 0.2)
	{
		sim::Vector3 const & observer_pos = observer.GetPosition();
		double observer_position_length = Length(observer_pos - scene.GetOrigin());
		gfx::Debug::out << "oor:" << max_observer_position_length - observer_position_length << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .75)
	{
		gfx::Debug::out << "scene_t:" << scene_tick_period << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .2)
	{
		if (PostResetFreeze())
		{
			gfx::Debug::out << "resetting...\n";
		}
	}
	
	if (gfx::Debug::GetVerbosity() > .15) 
	{
		gfx::Debug::out << "polys:" << mesh.GetNumPolys() << '\n';
	}
}

void form::SceneThread::ForEachFormation(FormationFunctor & f) const
{
	scene.ForEachFormation(f);
}

bool form::SceneThread::PollMesh(form::MeshBufferObject & mbo, sim::Vector3 & origin, bool & flat_shaded)
{
	Assert(IsMainThread());
	
	if (! mesh_mutex.TryLock())
	{
		return false;
	}
		
	bool polled;
	
	if (mesh_updated)
	{
		origin = mesh_origin;
		flat_shaded = mesh.GetFlatShaded();
		
		mbo.Set(mesh);

		mesh_updated = false;
		polled = true;
	}
	else
	{
		polled = false;
	}
	
	mesh_mutex.Unlock();
	
	return polled;
}

void form::SceneThread::ResetOrigin()
{
	Assert(IsMainThread());

	if (dynamic_origin)
	{
		reset_origin_flag = true;
	}
}

bool form::SceneThread::PostResetFreeze() const
{
	app::TimeType t = app::GetTime();
	app::TimeType time_since_reset = t - origin_reset_time;
	return time_since_reset < post_reset_freeze_period;
}

void form::SceneThread::ToggleFlatShaded()
{
	mesh.SetFlatShaded(! mesh.GetFlatShaded());
}

// TODO: Should really say 'need origin reset'
bool form::SceneThread::OutOfRange() const
{
	if (reset_origin_flag) 
	{
		// We're on it already!
		return false;
	}
	
	if (PostResetFreeze())
	{
		// Still making the last one
		return false;
	}
	
	sim::Vector3 const & observer_pos = observer.GetPosition();
	double observer_position_length = Length(observer_pos - scene.GetOrigin());
	return observer_position_length > max_observer_position_length;
}

void form::SceneThread::ThreadFunc(SceneThread * scene_thread)
{
	scene_thread->Run();
}

// The main loop of the scene thread. Hopefully it's pretty self-explanatory.
void form::SceneThread::Run()
{
	while (! quit_flag) 
	{
		if (suspend_flag) 
		{
			app::Sleep();
		}
		else 
		{
			ThreadTick();
		}
	}
	
	quit_flag = false;
}

// The tick function of the scene thread. 
// This functions gets called repeatedly either in the scene thread - if there is on -
// or in the main thread as part of the main render/simulation iteration.
void form::SceneThread::ThreadTick()
{
	sim::Ray3 camera_ray = observer.GetCameraRay();
	scene.SetCameraRay(camera_ray);
	
	if (reset_origin_flag) 
	{
		sim::Vector3 const & observer_pos = observer.GetPosition();
		scene.SetOrigin(observer_pos);
		reset_origin_flag = false;
		origin_reset_time = app::GetTime();
	}
	else 
	{
		AdjustNumQuaterna();
	}

	app::TimeType scene_tick_time = app::GetTime();
	scene.Tick(formations);
	scene_tick_period = app::GetTime() - scene_tick_time;

	GenerateMesh();
}

void form::SceneThread::AdjustNumQuaterna()
{
	if (PostResetFreeze() || frame_ratio < 0)
	{
		return;
	}
	
	// Attenuate/invert the frame_ratio.
	// Thus is becomes a multiplier on the new number of nodes.
	// A worse frame rate translates into a lower number of nodes
	// and hopefully, things improve. 
	float frame_ratio_log = Log(frame_ratio);
	float frame_ratio_exp = Exp(frame_ratio_log * - frame_rate_reaction_coefficient);

	// Do the same thing with the mesh generation period.
	// But only if we're over our prescribed time limit.
	if (mesh_generation_period > max_mesh_generation_period)
	{
		// And be a lot more blunt.
		frame_ratio_exp = Min(frame_ratio_exp, max_mesh_generation_reaction_coefficient);
		
		// Reset this so there isn't a continuous decrease in node-count before the next recording.
		mesh_generation_period = 0;
	}
	
	float num_quaterna = static_cast<float>(scene.GetNumQuaternaUsed());
	int target_num_quaterna = static_cast<int>(num_quaterna * frame_ratio_exp) + 1;
	
	scene.SetNumQuaternaUsedTarget(target_num_quaterna);
	
	// reset this in case thread ticks before the next frame happens.
	frame_ratio = -1;
}

bool form::SceneThread::GenerateMesh()
{
	//Assert(IsSceneThread());
	
	if (PostResetFreeze())
	{
		return false;
	}
	
	if (mesh_updated)
	{
		// No point making multiple meshes if main thread isn't polling them fast enough.
		return false;
	}
	
	if (! mesh_mutex.TryLock())
	{
		// Don't wait around if current mesh is being read by main thread;
		// This thread should be locked as little as possible.
		return false;
	}
	
	scene.GenerateMesh(mesh);
	mesh_origin = scene.GetOrigin();
	mesh_mutex.Unlock();

	mesh_updated = true;
	app::TimeType t = app::GetTime();
	mesh_generation_period = t - mesh_generation_time;
	mesh_generation_time = t;
	return true;
}

bool form::SceneThread::IsMainThread() const
{
	return ! threaded || ! (thread != nullptr && thread->IsCurrent());
}

bool form::SceneThread::IsSceneThread() const
{
	return ! threaded || (thread != nullptr && thread->IsCurrent());
}

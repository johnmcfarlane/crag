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
		thread = new Thread(* this);
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
}

void form::SceneThread::ForEachFormation(FormationFunctor & f) const
{
	scene.ForEachFormation(f);
}

bool form::SceneThread::PollMesh(form::MeshBufferObject & mbo)
{
	Assert(IsMainThread());
	
	if (! mesh_mutex.TryLock())
	{
		return false;
	}
		
	bool polled;
	
	if (mesh_updated)
	{
		mbo.Set(mesh, mesh_origin, mesh.GetFlatShaded());

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

// Returns false if a new origin is needed.
bool form::SceneThread::IsOriginOk() const
{
	if (reset_origin_flag) 
	{
		// We're on it already!
		return true;
	}
	
	if (PostResetFreeze())
	{
		// Still making the last one
		return true;
	}
	
	// The real test: Is the observer not far enough away from the 
	// current origin that visible inaccuracies might become apparent?
	sim::Vector3 const & observer_pos = observer.GetPosition();
	double observer_position_length = Length(observer_pos - scene.GetOrigin());
	return observer_position_length < max_observer_position_length;
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
	
	// Come up with two accounts of how many quaterna we should have.
	int current_num_quaterna = scene.GetNumQuaternaUsed();
	int frame_ratio_directed_target_num_quaterna = CalculateFrameRateDirectedTargetNumQuaterna(current_num_quaterna, frame_ratio);
	int mesh_generation_directed_target_num_quaterna = CalculateMeshGenerationDirectedTargetNumQuaterna(current_num_quaterna, static_cast<float>(mesh_generation_period));

	// Pick the more conservative and submit it.
	int target_num_quaterna = Min(mesh_generation_directed_target_num_quaterna, frame_ratio_directed_target_num_quaterna);
	scene.SetNumQuaternaUsedTarget(target_num_quaterna);
	
	// Reset the parameters used to come to a decision so
	// we don't just keep acting on them over and over. 
	mesh_generation_period = 0;
	frame_ratio = -1;
}

// Taking into account the framerate ratio, come up with a quaterna count.
int form::SceneThread::CalculateFrameRateDirectedTargetNumQuaterna(int current_num_quaterna, float frame_ratio) 
{
	// Attenuate/invert the frame_ratio.
	// Thus is becomes a multiplier on the new number of nodes.
	// A worse frame rate translates into a lower number of nodes
	// and hopefully, things improve. 
	float frame_ratio_log = Log(frame_ratio);
	float frame_ratio_exp = Exp(frame_ratio_log * - frame_rate_reaction_coefficient);
	
	float num_quaterna = static_cast<float>(current_num_quaterna);	
	
	int frame_ratio_directed_target_num_quaterna = static_cast<int>(num_quaterna * frame_ratio_exp) + 1;
	
	// Ensure the value is suitably clamped. 
	if (frame_ratio_directed_target_num_quaterna > NodeBuffer::max_num_quaterna)
	{
		frame_ratio_directed_target_num_quaterna = NodeBuffer::max_num_quaterna;
	}
	Assert(frame_ratio_directed_target_num_quaterna > 0);
	
	return frame_ratio_directed_target_num_quaterna;
}

// Taking into account how long it took to generate the last mesh for the renderer, come up with a quaterna count.
int form::SceneThread::CalculateMeshGenerationDirectedTargetNumQuaterna(int current_num_quaterna, float mesh_generation_period)
{
	// We're under budget so everything's ok.
	if (mesh_generation_period < max_mesh_generation_period)
	{
		return NodeBuffer::max_num_quaterna;
	}
	
	// Reset this so there isn't a continuous decrease in node-count before the next recording.
	int mesh_generation_directed_target_num_quaterna = static_cast<int>(current_num_quaterna * max_mesh_generation_reaction_coefficient) - 1;
	if (mesh_generation_directed_target_num_quaterna < 0)
	{
		mesh_generation_directed_target_num_quaterna = 0;
	}
	
	return mesh_generation_directed_target_num_quaterna;
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

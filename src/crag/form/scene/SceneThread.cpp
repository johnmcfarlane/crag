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
	CONFIG_DEFINE (max_mesh_generation_period, sys::TimeType, .2f);
	CONFIG_DEFINE (post_reset_freeze_period, sys::TimeType, 1.25f);
	CONFIG_DEFINE (frame_rate_reaction_coefficient, float, 0.01f);
	CONFIG_DEFINE (max_mesh_generation_reaction_coefficient, float, 0.9975f);	// Multiply node count by this number when mesh generation is too slow.
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
	
	if (threaded) 
	{
		Assert(thread == nullptr);
		thread = new Thread(* this);
	}
}

void form::SceneThread::ToggleSuspended()
{
	suspend_flag = ! suspend_flag;
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

void form::SceneThread::Tick()
{
	Assert(IsMainThread());

	if (! threaded) 
	{
		TickThread();
	}
	
	if (gfx::Debug::GetVerbosity() > 0.2)
	{
		sim::Vector3 const & observer_pos = observer.GetPosition();
		double observer_position_length = Length(observer_pos - GetActiveScene().GetOrigin());
		gfx::Debug::out << "oor:" << max_observer_position_length - observer_position_length << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .25)
	{
		gfx::Debug::out << "scene_t:" << scene_tick_period << '\n';
	}
	
	if (gfx::Debug::GetVerbosity() > .37) 
	{
		gfx::Debug::out << "visible_q:" << GetVisibleScene().GetNumQuaternaUsed() << '\n';
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

bool form::SceneThread::PollMesh(form::MeshBufferObject & mbo)
{
	Assert(IsMainThread());
	
	if (! mesh_semaphore.TryLock())
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
	
	mesh_semaphore.Unlock();
	
	return polled;
}

void form::SceneThread::ResetOrigin()
{
	Assert(IsMainThread());

	reset_origin_flag = true;
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
	while (! quit_flag) 
	{
		if (suspend_flag) 
		{
			sys::Sleep();
		}
		else 
		{
			TickThread();
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
	
	sys::TimeType scene_tick_time = sys::GetTime();
	active_scene.Tick(formations);
	scene_tick_period = sys::GetTime() - scene_tick_time;
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
	if (IsResetting() || frame_ratio < 0)
	{
		return;
	}
	
	Scene & active_scene = GetActiveScene();
	
	// Come up with two accounts of how many quaterna we should have.
	int current_num_quaterna = active_scene.GetNumQuaternaUsed();
	int frame_ratio_directed_target_num_quaterna = CalculateFrameRateDirectedTargetNumQuaterna(current_num_quaterna, frame_ratio);
	int mesh_generation_directed_target_num_quaterna = CalculateMeshGenerationDirectedTargetNumQuaterna(current_num_quaterna, static_cast<float>(mesh_generation_period));

	// Pick the more conservative and submit it.
	int target_num_quaterna = Min(mesh_generation_directed_target_num_quaterna, frame_ratio_directed_target_num_quaterna);
	active_scene.SetNumQuaternaUsedTarget(target_num_quaterna);
	
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

void form::SceneThread::GenerateMesh()
{
	//Assert(IsSceneThread());
	if (IsGrowing())
	{
		return;
	}
	
	if (IsResetting())
	{
		return;
	}
	
	if (mesh_updated)
	{
		// No point making multiple meshes if main thread isn't polling them fast enough.
		return;
	}
	
	if (! mesh_semaphore.TryLock())
	{
		// Don't wait around if current mesh is being read by main thread;
		// This thread should be locked as little as possible.
		return;
	}
	
	Scene & visible_scene = GetVisibleScene();
	visible_scene.GenerateMesh(mesh);
	mesh_origin = visible_scene.GetOrigin();
	mesh_semaphore.Unlock();

	mesh_updated = true;
	sys::TimeType t = sys::GetTime();
	mesh_generation_period = t - mesh_generation_time;
	mesh_generation_time = t;
}

bool form::SceneThread::IsMainThread() const
{
	return ! threaded || ! (thread != nullptr && thread->IsCurrent());
}

bool form::SceneThread::IsSceneThread() const
{
	return ! threaded || (thread != nullptr && thread->IsCurrent());
}

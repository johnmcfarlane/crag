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

#include "form/SceneThread.h"
#include "form/Manager.h"

#include "sim/Observer.h"

#include "core/ConfigEntry.h"


namespace ANONYMOUS 
{
	CONFIG_DEFINE (max_observer_position_length, double, 5000);
	CONFIG_DEFINE (max_mesh_generation_period, float, .1f);
	CONFIG_DEFINE (post_reset_freeze_duration, float, 1.25f);
}


form::SceneThread::SceneThread(FormationSet const & _formations, sim::Observer const & _observer, bool _threaded)
: num_nodes(100)
, frame_ratio(-1)
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

int form::SceneThread::GetNumQuaternaAvailable() const
{
	return scene.GetNumQuaternaAvailable();
}

void form::SceneThread::Tick()
{
	Assert(IsMainThread());

	if (! threaded) {
		ThreadTick();
	}
}

bool form::SceneThread::PollMesh(form::MeshBufferObject & mbo, sim::Vector3 & mesh_origin)
{
	Assert(IsMainThread());
	
	bool polled;
	
	mesh_mutex.Lock();
	
	if (mesh_updated)
	{
		mbo.Set(mesh);

		mesh_origin = scene.GetOrigin();

		mesh_updated = false;
		polled = true;
	}
	else
	{
		polled = false;
	}
	
	mesh_mutex.Unlock();
	
	//app::Sleep(0);

	return polled;
}

void form::SceneThread::ResetOrigin()
{
	Assert(IsMainThread());

	reset_origin_flag = true;
}

bool form::SceneThread::PostResetFreeze() const
{
	app::TimeType t = app::GetTime();
	app::TimeType time_since_reset = t - origin_reset_time;
	float time_since_reset_seconds = app::TimeTypeToSeconds(time_since_reset);
	return time_since_reset_seconds < post_reset_freeze_duration;
}

bool form::SceneThread::OutOfRange() const
{
	if (reset_origin_flag) 
	{
		// We're on it already!
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

void form::SceneThread::Run()
{
//	Assert(IsSceneThread());
	
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

void form::SceneThread::ThreadTick()
{
	//Assert(IsSceneThread());

	sim::Vector3 const & observer_pos = observer.GetPosition();
	scene.SetObserverPos(observer_pos);
	
	if (reset_origin_flag) 
	{
		VerifyObject(* this);
		
		scene.SetOrigin(observer_pos);
		reset_origin_flag = false;
		origin_reset_time = app::GetTime();

		VerifyObject(* this);
	}
	else 
	{
		AdjustNumQuaterna();
	}

	scene.Tick(formations);
	GenerateMesh();
}

void form::SceneThread::AdjustNumQuaterna()
{
	if (PostResetFreeze() || frame_ratio < 0)
	{
		return;
	}
	
#if ! defined(NDEBUG)
	Assert(frame_ratio > 0);
	float test_in = 2.353;
	float test_out = Log(Exp(test_in));
	Assert(test_out == test_in);
#endif
	
	float num_quaterna = static_cast<float>(scene.GetNumQuaternaAvailable());
	float l = Log(frame_ratio);
	float e = Exp(l * -0.01f);
	
	if (mesh_generation_period > max_mesh_generation_period)
	{
		e = Min(e, .9f);
	}
	
	int target_num_quaterna = static_cast<int>(num_quaterna * e) + 1;
	
	scene.SetNumQuaternaAvailable(target_num_quaterna);
	
	// reset this in case thread ticks before the next frame happens.
	frame_ratio = -1;
}

bool form::SceneThread::GenerateMesh()
{
	Assert(IsSceneThread());
	
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
	mesh_mutex.Unlock();

	mesh_updated = true;
	app::TimeType t = app::GetTime();
	mesh_generation_period = app::TimeTypeToSeconds(t - mesh_generation_time);
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

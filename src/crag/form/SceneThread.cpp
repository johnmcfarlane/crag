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


namespace ANONYMOUS {

// TODO: Make this more precise / general purpose.
CONFIG_DEFINE (max_observer_position_length, double, 5000);

}


form::SceneThread::SceneThread(FormationSet const & _formations, sim::Observer const & _observer, bool _threaded)
: num_nodes(100)
, frame_ratio(1)
, formations(_formations)
, observer(_observer)
, threaded(_threaded)
, reset_origin_flag(false)
, suspend_flag(false)
, quit_flag(false)
, mesh(static_cast<int>(form::NodeBuffer::max_num_verts * 1.25f))
, mesh_updated(false)
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
	if (threaded) {
		Assert(thread == nullptr);
		thread = new core::Thread(ThreadFunc, this);
	}
}

void form::SceneThread::Quit()
{
	quit_flag = true;

	if (thread)
	{
		thread->Join();
	}
}

/*int form::SceneThread::GetTargetNumNodes() const
{
	return target_num_nodes;
}*/

void form::SceneThread::SetFrameRatio(float ratio)
{
	frame_ratio = Max(ratio, std::numeric_limits<float>::min());
}

int form::SceneThread::GetNumQuaternaAvailable() const
{
	return scene.GetNumQuaternaAvailable();
}

void form::SceneThread::Tick()
{
	if (! threaded) {
		ThreadTick();
	}
}

bool form::SceneThread::PollMesh(form::MeshBufferObject & mbo, sim::Vector3 & mesh_origin)
{
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
	
//	GLPP_CALL(glFinish());
//	GLPP_CALL(glFlush());
	mesh_mutex.Unlock();
	
	app::Sleep(0);

	return polled;
}

void form::SceneThread::ResetOrigin()
{
	reset_origin_flag = true;
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
	while (! quit_flag) {
		if (suspend_flag) {
			app::Sleep();
		}
		else {
			ThreadTick();
		}
	}
	
	quit_flag = false;
}

void form::SceneThread::ThreadTick()
{
	sim::Vector3 const & observer_pos = observer.GetPosition();
	scene.SetObserverPos(observer_pos);
		
	bool generate = false;	
	if (reset_origin_flag) 
	{
		generate = false;

		VerifyObject(* this);
		
		reset_origin_flag = false;
		scene.SetOrigin(observer_pos);

		VerifyObject(* this);
	}
	else 
	{
		AdjustNumQuaterna();

		generate = true;
	}

	scene.Tick(formations);

	if (generate) 
	{
		GenerateMesh();
	}
}

void form::SceneThread::AdjustNumQuaterna()
{
#if ! defined(NDEBUG)
	Assert(frame_ratio > 0);
	float test_in = 2.353;
	float test_out = Log(Exp(test_in));
	Assert(test_out == test_in);
#endif
	
	float num_quaterna = static_cast<float>(scene.GetNumQuaternaAvailable());
	float l = Log(frame_ratio);
	float e = Exp(l * -0.01f);
	
	int target_num_quaterna = static_cast<int>(num_quaterna * e) + 1;
	
	scene.SetNumQuaternaAvailable(target_num_quaterna);
	
	// reset this in case thread ticks before the next frame happens.
	frame_ratio = 1;
}

bool form::SceneThread::GenerateMesh()
{
	if (mesh_mutex.TryLock())
	{
		scene.GenerateMesh(mesh);
		mesh_updated = true;
		mesh_mutex.Unlock();
		return true;
	}
	
	return false;
}


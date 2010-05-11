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
/*#if __APPLE__
	if (threaded) {
		CGLContextObj main_thread_render_context = CGLGetCurrentContext();
		CGLError err = CGLCreateContext(
										CGLGetPixelFormat(main_thread_render_context),
										main_thread_render_context,
										& render_context);
		
		if (err != kCGLNoError) {
			threaded = false;
		}
	}
#endif*/
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
		thread = new boost::thread(ThreadFunc, this);
	}
}

void form::SceneThread::Quit()
{
	quit_flag = true;

	if (thread)
	{
		thread->join();
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

	mesh_mutex.lock();
	if (mesh_updated)
	{
		mbo.Set(mesh);

		//scene_mutex.lock();
		mesh_origin = scene.GetOrigin();

		//if (reset_origin) {
		//	scene.SetOrigin(observer.GetPosition());
		//}
		//scene_mutex.unlock();

		mesh_updated = false;
		polled = true;
	}
	else
	{
		polled = false;
	}
	mesh_mutex.unlock();

	return polled;
}

void form::SceneThread::ResetOrigin()
{
	reset_origin_flag = true;
}

bool form::SceneThread::OutOfRange() const
{
	return false;
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
			boost::thread::yield();
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
	
	AdjustNumQuaterna();
	scene.SetObserverPos(observer_pos);
	scene.Tick(formations);
	
	bool generate = false;
	
	if (reset_origin_flag) {
		reset_origin_flag = false;
		scene.SetOrigin(observer_pos);
		generate = true;
	}
	
	/*if (scene.GetChurnMetric() > -.05f)*/ {
		generate = true;
	}

	if (generate) {
		//scene.ResetChurnCounter();
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
	
	// reset this dude in case thread ticks before the next frame happens.
	frame_ratio = 1;
}

void form::SceneThread::GenerateMesh()
{
	mesh_mutex.lock();
	scene.GenerateMesh(mesh);
	mesh_updated = true;
	mesh_mutex.unlock();
}


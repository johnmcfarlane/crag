//
//  ScriptThread.cpp
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ScriptThread.h"
#include "MainFunctor.h"

#include "sim/Entity.h"
#include "sim/Simulation.h"

#include "gfx/Renderer.h"

#include "geom/Transformation.h"

#include "core/app.h"


#if defined(NDEBUG)
#define FILE_LOCAL_BEGIN namespace {
#define FILE_LOCAL_END }
#else
#define FILE_LOCAL_BEGIN 
#define FILE_LOCAL_END 
#endif


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// ScriptThread member definitions

ScriptThread::ScriptThread()
: _time(0)
, _unlaunched_fiber(nullptr)
, _quit_flag(false)
{
	smp::SetThreadPriority(1);
	smp::SetThreadName("Script");
}

ScriptThread::~ScriptThread()
{
	Assert(_fibers.empty());
	Assert(_unlaunched_fiber == nullptr);
	
	Assert(_quit_flag);
}

void ScriptThread::OnQuit()
{
	SetQuitFlag();
	SDL_Event quit_event = { SDL_QUIT };
	OnEvent(quit_event);
}

void ScriptThread::OnEvent(SDL_Event const & event)
{
	_events.push(event);
}

bool ScriptThread::GetQuitFlag() const
{
	return _quit_flag;
}

void ScriptThread::SetQuitFlag()
{
	_quit_flag = true;
}

Time ScriptThread::GetTime() const
{
	return _time;
}

void ScriptThread::SetTime(Time const & time)
{
	_time = time;
}

// Note: Run should be called from same thread as c'tor/d'tor.
void ScriptThread::Run(Daemon::MessageQueue & message_queue)
{
	// Launch the main script.
	MainFunctor main_functor;
	FiberEntry<MainFunctor> & main_fiber = ref(new FiberEntry<MainFunctor>(main_functor));
	Launch(main_fiber);

	// 
	while (HasFibersActive())
	{
		ContinueTask();

		message_queue.DispatchMessages(* this);
	}
	
	message_queue.DispatchMessages(* this);
}

void ScriptThread::GetEvent(SDL_Event & event)
{
	if (_events.empty())
	{
		event.type = 0;
		return;
	}
	
	event = _events.front();
	_events.pop();
}

void ScriptThread::Launch(Fiber & fiber)
{
	Assert(_unlaunched_fiber == nullptr);
	_unlaunched_fiber = & fiber;
}

bool ScriptThread::HasFibersActive() const
{
	return _unlaunched_fiber != nullptr || ! _fibers.empty();
}

void ScriptThread::ContinueTask()
{
	Assert(HasFibersActive());
	
	// pick the next fiber to process
	Fiber * fiber;
	if (_unlaunched_fiber != nullptr)
	{
		Assert(! _fibers.contains(* _unlaunched_fiber));
		
		fiber = _unlaunched_fiber;
		_unlaunched_fiber = nullptr;
		fiber->Start(* this);
	}
	else if (! _fibers.empty())
	{
		fiber = & _fibers.front();
		_fibers.pop_front();
		
		if (fiber->IsComplete())
		{
			delete fiber;
			return;
		}
		
		if (fiber->TestCondition(* this))
		{
			fiber->Continue();
		}
	}
	
	_fibers.push_back(* fiber);
}

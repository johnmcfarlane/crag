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

#include "Script.h"


#if defined(NDEBUG)
#define FILE_LOCAL_BEGIN namespace {
#define FILE_LOCAL_END }
#else
#define FILE_LOCAL_BEGIN 
#define FILE_LOCAL_END 
#endif


using namespace script;


namespace
{
	Fiber * FindFiber(Uid uid, Fiber::List & list)
	{
		for (auto i = list.begin(), end = list.end(); i != end; ++ i)
		{
			Fiber & fiber = * i;
			
			Script & script = fiber.GetScript();
			if (script.GetUid() != uid)
			{
				continue;
			}
			
			return & fiber;
		}
		
		return nullptr;	
	}
}


////////////////////////////////////////////////////////////////////////////////
// ScriptThread member definitions

ScriptThread::ScriptThread()
: _time(0)
, _quit_flag(false)
{
	smp::SetThreadPriority(1);
}

ScriptThread::~ScriptThread()
{
	ASSERT(_fibers.empty());
	ASSERT(_unlaunched_fibers.empty());
	
	ASSERT(_quit_flag);
}

Script * ScriptThread::GetObject(Uid uid)
{
	Fiber * fiber;
	
	fiber = FindFiber(uid, _fibers);
	if (fiber == nullptr)
	{
		fiber = FindFiber(uid, _unlaunched_fibers);
		if (fiber == nullptr)
		{
			return nullptr;
		}
	}
	
	return & fiber->GetScript();
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

void ScriptThread::OnAddObject(Script & script)
{
	if (! script.GetUid())
	{
		script.SetUid(Uid::Create());
	}
	
	Fiber * fiber = new Fiber(script);
	_unlaunched_fibers.push_back(ref(fiber));
}

void ScriptThread::OnRemoveObject(Uid const & uid)
{
	Fiber * fiber;
	
	fiber = FindFiber(uid, _fibers);
	if (fiber != nullptr)
	{
		_fibers.remove(* fiber);
	}
	else
	{
		fiber = FindFiber(uid, _unlaunched_fibers);
		if (fiber != nullptr)
		{
			_unlaunched_fibers.remove(* fiber);
		}
		else
		{
			// Presumably, the object has already been removed.
			// This could happen during shutdown for instance.
			return;
		}
	}

	ASSERT(fiber->IsComplete());
	delete fiber;
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
	// Wait around until there are scripts to run.
	while (! HasFibersActive())
	{
		if (message_queue.DispatchMessages(* this) == 0)
		{
			smp::Yield();
		}
	}
	
	// Main script loop.
	while (HasFibersActive())
	{
		bool dispatched_messages = message_queue.DispatchMessages(* this) != 0;
		bool processed_tasks = ProcessTasks();
		if (! (dispatched_messages | processed_tasks))
		{
			smp::Yield();
		}
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

bool ScriptThread::HasFibersActive() const
{
	return ! _fibers.empty() || ! _unlaunched_fibers.empty();
}

bool ScriptThread::ProcessTasks()
{
	ASSERT(HasFibersActive());
	
	return StartTask() || ContinueTask();
}

bool ScriptThread::StartTask()
{
	// pick the next fiber to process
	if (_unlaunched_fibers.empty())
	{
		return false;
	}
	
	Fiber & fiber = _unlaunched_fibers.front();
	_unlaunched_fibers.pop_front();

	fiber.Start(* this);
	
	ASSERT(! _fibers.contains(fiber));
	_fibers.push_back(fiber);
	
	return true;
}

bool ScriptThread::ContinueTask()
{
	if (_fibers.empty())
	{
		return false;
	}
	
	Fiber & first = _fibers.front();
	Fiber * fiber = & first;
	do
	{
		ASSERT(! fiber->IsComplete());

		_fibers.pop_front();
		_fibers.push_back(* fiber);
		
		Condition & condition = ref(fiber->GetCondition());
		if (condition(* this))
		{
			fiber->Continue();

			if (fiber->IsComplete())
			{
				_fibers.remove(* fiber);
				delete fiber;
			}
			
			return true;
		}
		
		fiber = & _fibers.front();
	}	while (fiber != & first);
	
	return false;
}

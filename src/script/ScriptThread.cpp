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

#include "Condition.h"


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
, _quit_flag(false)
{
	smp::SetThreadPriority(1);
}

ScriptThread::~ScriptThread()
{
	ASSERT(_scripts.empty());
	
	ASSERT(_quit_flag);
}

Script * ScriptThread::GetObject(Uid uid)
{
	for (auto i = _scripts.begin(), end = _scripts.end(); i != end; ++ i)
	{
		Script & script = * i;
		if (script.GetUid() == uid)
		{
			return & script;
		}
	}
	
	return nullptr;
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
	
	_scripts.push_back(script);
	script.SetScriptThread(* this);
}

void ScriptThread::OnRemoveObject(Uid const & uid)
{
	Script * script = GetObject(uid);
	
	if (script != nullptr)
	{
		ASSERT(! script->IsRunning());
		_scripts.remove(* script);
		delete script;
	}
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
	return ! _scripts.empty();
}

bool ScriptThread::ProcessTasks()
{
	ASSERT(HasFibersActive());
	
	if (_scripts.empty())
	{
		return false;
	}
	
	Script & first = _scripts.front();
	Script * script = & first;
	do
	{
		ASSERT(script->IsRunning());

		_scripts.pop_front();
		_scripts.push_back(* script);
		
		Condition & condition = ref(script->GetCondition());
		if (condition(* this))
		{
			script->Continue();

			if (! script->IsRunning())
			{
				_scripts.remove(* script);
				delete script;
			}
			
			return true;
		}
		
		script = & static_cast<Script &>(_scripts.front());
	}	while (script != & first);
	
	return false;
}

//
//  script/Engine.cpp
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Engine.h"

#include "Condition.h"
#include "ScriptBase.h"


#if defined(NDEBUG)
#define FILE_LOCAL_BEGIN namespace {
#define FILE_LOCAL_END }
#else
#define FILE_LOCAL_BEGIN 
#define FILE_LOCAL_END 
#endif


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// Engine member definitions

Engine::Engine()
: _time(0)
, _quit_flag(false)
{
	smp::SetThreadPriority(1);
}

Engine::~Engine()
{
	ASSERT(_scripts.empty());
	
	ASSERT(_quit_flag);
}

ScriptBase * Engine::GetObject(Uid uid)
{
	for (auto i = _scripts.begin(), end = _scripts.end(); i != end; ++ i)
	{
		ScriptBase & script = * i;
		if (script.GetUid() == uid)
		{
			return & script;
		}
	}
	
	return nullptr;
}

void Engine::OnQuit()
{
	SetQuitFlag();
}

void Engine::OnAddObject(ScriptBase * const & script)
{
	if (! script->GetUid())
	{
		script->SetUid(Uid::Create());
	}
	
	_scripts.push_back(* script);
}

void Engine::OnRemoveObject(Uid const & uid)
{
	ScriptBase * script = GetObject(uid);
	
	if (script != nullptr)
	{
		ASSERT(! script->IsRunning());
		_scripts.remove(* script);
		delete script;
	}
}

void Engine::SetQuitFlag()
{
	_quit_flag = true;
	
	for (auto i = _scripts.begin(), end = _scripts.end(); i != end; ++ i)
	{
		FiberInterface & script = * i;
		script.SetQuitFlag();
	}
}

// Note: Run should be called from same thread as c'tor/d'tor.
void Engine::Run(Daemon::MessageQueue & message_queue)
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

bool Engine::HasFibersActive() const
{
	return ! _scripts.empty();
}

bool Engine::ProcessTasks()
{
	ASSERT(HasFibersActive());
	
	if (_scripts.empty())
	{
		return false;
	}
	
	ScriptBase & first = _scripts.front();
	ScriptBase * script = & first;
	do
	{
		ASSERT(script->IsRunning());

		_scripts.pop_front();
		_scripts.push_back(* script);
		
		Condition & condition = ref(script->GetCondition());
		if (condition())
		{
			script->Continue();

			if (! script->IsRunning())
			{
				_scripts.remove(* script);
				delete script;
			}
			
			return true;
		}
		
		script = & static_cast<ScriptBase &>(_scripts.front());
	}	while (script != & first);
	
	return false;
}

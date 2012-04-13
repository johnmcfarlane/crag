//
//  applet/Engine.cpp
//  crag
//
//  Created by John McFarlane on 1/19/11.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Engine.h"

#include "Condition.h"
#include "AppletBase.h"


#if defined(NDEBUG)
#define FILE_LOCAL_BEGIN namespace {
#define FILE_LOCAL_END }
#else
#define FILE_LOCAL_BEGIN 
#define FILE_LOCAL_END 
#endif


using namespace applet;


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
	ASSERT(_applets.empty());
	
	ASSERT(_quit_flag);
}

AppletBase * Engine::GetObject(Uid uid)
{
	for (auto i = _applets.begin(), end = _applets.end(); i != end; ++ i)
	{
		AppletBase & applet = * i;
		if (applet.GetUid() == uid)
		{
			return & applet;
		}
	}
	
	return nullptr;
}

void Engine::OnQuit()
{
	SetQuitFlag();
}

void Engine::OnAddObject(AppletBase * const & applet)
{
	if (! applet->GetUid())
	{
		applet->SetUid(Uid::Create());
	}
	
	// If engine's quit flag is set,
	if (_quit_flag)
	{
		DEBUG_MESSAGE("quit flag already set; deleting applet");
		
		// delete it.
		delete applet;
		return;
	}

	_applets.push_back(* applet);
}

void Engine::OnRemoveObject(Uid const & uid)
{
	AppletBase * applet = GetObject(uid);
	
	if (applet != nullptr)
	{
		ASSERT(! applet->IsRunning());
		_applets.remove(* applet);
		delete applet;
	}
}

void Engine::SetQuitFlag()
{
	_quit_flag = true;
	
	for (auto i = _applets.begin(), end = _applets.end(); i != end; ++ i)
	{
		AppletBase & applet = * i;
		applet.SetQuitFlag();
	}
}

// Note: Run should be called from same thread as c'tor/d'tor.
void Engine::Run(Daemon::MessageQueue & message_queue)
{
	// Wait around until there are applets to run.
	while (! HasFibersActive())
	{
		if (message_queue.DispatchMessages(* this) == 0)
		{
			smp::Yield();
		}
	}
	
	// Main loop.
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
	return ! _applets.empty();
}

// Give all applets an opportunity to run.
bool Engine::ProcessTasks()
{
	bool did_work = false;
	
	// Step through all applets,
	for (auto i = _applets.begin(), end = _applets.end(); i != end; ++ i)
	{
		AppletBase & applet = * i;

		ASSERT(applet.IsRunning());

		// and if applet's continue condition is met,
		Condition & condition = ref(applet.GetCondition());
		if (condition(_quit_flag))
		{
			// then continue!
			applet.Continue();
			
			// When it pauses again, 
			did_work = true;

			// And if it's all done, 
			if (! applet.IsRunning())
			{
				// take a step back,
				-- i;
				
				// and remove the applet.
				_applets.remove(applet);
				delete & applet;
			}
		}
	}
	
	// Return true iff any happened.
	return did_work;
}

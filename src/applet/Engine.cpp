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

#include "AppletBase.h"

#include "smp/Fiber.h"


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
}

Engine::~Engine()
{
	ASSERT(_applets.empty());
	
	ASSERT(_quit_flag);
}

void Engine::OnQuit()
{
	SetQuitFlag();
}

void Engine::SetQuitFlag()
{
	_quit_flag = true;
}

// Note: Run should be called from same thread as c'tor/d'tor.
void Engine::Run(Daemon::MessageQueue & message_queue)
{
	smp::Fiber::InitializeThread();

	// Main loop.
	while (message_queue.DispatchMessages(* this) != 0 || HasFibersActive() || ! _quit_flag)
	{
		if (! ProcessTasks())
		{
			smp::Sleep(0.01);
		}
	}
}

bool Engine::HasFibersActive() const
{
	return GetNumObjects() != 0;
}

// Give all applets an opportunity to run.
bool Engine::ProcessTasks()
{
	bool did_work = false;
	
	// Step through all applets,
	ForEachObject_Destroy([this, & did_work] (AppletBase & applet) -> bool 
	{
		// If work was not done, 
		if (! ProcessTask(applet))
		{
			return true;
		}

		did_work = true;

		// And if it's all done, 
		if (! applet.IsRunning())
		{
			// destroy the applet.
			return false;
		}
		else if (_quit_flag)
		{
			DEBUG_MESSAGE("Warning: %s told to quit but did not exit", applet.GetName());
		}

		return true;
	});
	
	// Return true iff any happened.
	return did_work;
}

bool Engine::ProcessTask(AppletBase & applet)
{
	ASSERT(applet.IsRunning());

	if (_quit_flag)
	{
		applet.SetQuitFlag();
	}
	
	const Condition & condition = applet.GetCondition();
	if (! condition())
	{
		if (_quit_flag)
		{
			DEBUG_MESSAGE("Warning: %s told to quit but did not wake", applet.GetName());
		}

		return false;
	}
	
	// and if applet's continue condition is met, then continue!
	// TODO: This uses as much as twice the number of context switches necessary 
	// to visit all applets. One applet should be able to switch straight on 
	// to the next.
	applet.Continue();
	return true;
}

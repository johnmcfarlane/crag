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

#include "Applet.h"

#include "ipc/Fiber.h"


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
: _quit_flag(false)
{
}

Engine::~Engine()
{
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
	ipc::Fiber::InitializeThread();

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
	return ! IsEmpty();
}

// Give all applets an opportunity to run.
bool Engine::ProcessTasks()
{
	bool did_work = false;
	
	// Step through all applets,
	ForEachObject_DestroyIf([this, & did_work] (Applet & applet) -> bool 
	{
		// If processing was not done, 
		if (! ProcessTask(applet))
		{
			// applet hasn't changed state.
			return false;
		}

		did_work = true;

		// And if it's all done, 
		if (! applet.IsRunning())
		{
			// the last applet quitting means it's time for the engine 
			// (and subsequently the program) to quit also
			_quit_flag = true;

			// destroy the applet.
			return true;
		}
		else if (_quit_flag)
		{
#if ! defined(NDEBUG)
			if (ipc::ListenerBase::CanExit())
			{
				DEBUG_MESSAGE("Warning: %s told to quit but did not exit", applet.GetName());
			}
#endif
		}

		return false;
	});
	
	// Return true iff any happened.
	return did_work;
}

bool Engine::ProcessTask(Applet & applet)
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
#if ! defined(NDEBUG)
			if (ipc::ListenerBase::CanExit())
			{
				DEBUG_BREAK("Warning: %s told to quit but did not wake", applet.GetName());
			}
#endif
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

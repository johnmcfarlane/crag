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

void Engine::OnAddObject(AppletBase & applet)
{
	_applets.push_back(applet);
}

void Engine::OnRemoveObject(Uid uid)
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
		if (! ProcessTask(applet))
		{
			continue;
		}
		
		// If work was done, 
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
		else if (_quit_flag)
		{
			DEBUG_MESSAGE("Warning: %s told to quit but did not exit", applet.GetName());
		}
	}
	
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

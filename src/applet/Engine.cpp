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

#include "core/ConfigEntry.h"

#if defined(NDEBUG)
#define FILE_LOCAL_BEGIN namespace {
#define FILE_LOCAL_END }
#else
#define FILE_LOCAL_BEGIN 
#define FILE_LOCAL_END 
#endif


using namespace applet;

namespace
{
	CONFIG_DEFINE(applet_min_interval, core::Time, 0.02);
}

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
	do
	{
		auto next_wake_time = ProcessTasks();
		if (next_wake_time < 0)
		{
			message_queue.DispatchMessage(* this);
			continue;
		}
		
		auto pause_time = std::max(applet_min_interval, next_wake_time - app::GetTime());
		if (message_queue.TryDispatchMessage(* this, pause_time))
		{
			message_queue.DispatchMessages(* this);
		}
	}
	while (! _quit_flag || HasFibersActive());
}
 
bool Engine::HasFibersActive() const
{
	return ! IsEmpty();
}

// gives the next Applet an opportunity to run;
// returns time when next Applet should run
core::Time Engine::ProcessTasks()
{
	Applet * soonest = nullptr;
	auto min_wake_time = std::numeric_limits<core::Time>::max();
	
	ForEachObject([&] (Applet & applet) {
		ASSERT(applet.IsRunning());
		
		auto wake_time = applet.GetWakeTime();
		if (wake_time < min_wake_time)
		{
			min_wake_time = wake_time;
			soonest = & applet;
		}
	});
	
	if (! soonest)
	{
		// there are no Applets
		return -1;
	}
	
	return ProcessTask(* soonest);
}

core::Time Engine::ProcessTask(Applet & applet)
{
	if (_quit_flag)
	{
		applet.SetQuitFlag();
	}
	else
	{
		auto wake_time = applet.GetWakeTime();
		if (wake_time > app::GetTime())
		{
			return wake_time;
		}
	}
	
	applet.Continue();
	
	if (! applet.IsRunning())
	{
		ReleaseObject(applet);
	}
	
	return 0;
}

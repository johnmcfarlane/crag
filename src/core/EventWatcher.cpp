//
//  EventWatcher.cpp
//  crag
//
//  Created by John McFarlane on 2/17/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "EventWatcher.h"

#include "smp/smp.h"

#include "core/app.h"


using namespace core;


////////////////////////////////////////////////////////////////////////////////
// core::EventWatcher member definitions

EventWatcher::EventWatcher()
{
	SDL_AddEventWatch(OnEvent, this);
}

EventWatcher::~EventWatcher()
{
	SDL_DelEventWatch(OnEvent, this);
}

bool EventWatcher::IsEmpty()
{
	std::lock_guard<smp::SimpleMutex> lock(_mutex);
	
	return _events.empty();
}

bool EventWatcher::PopEvent(SDL_Event & event)
{
	std::lock_guard<smp::SimpleMutex> lock(_mutex);
	if (_events.empty())
	{
		return false;
	}
	
	event = _events.back();
	_events.pop_back();
	return true;
}

int EventWatcher::OnEvent(void * userdata, SDL_Event * event)
{
	auto & event_watcher = ref(reinterpret_cast<EventWatcher *>(userdata));
	
	std::lock_guard<smp::SimpleMutex> lock(event_watcher._mutex);
	event_watcher._events.push_back(* event);

	return 0;
}

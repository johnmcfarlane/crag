//
//  EventCondition.cpp
//  crag
//
//  Created by John McFarlane on 2/17/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "EventCondition.h"

#include "smp/smp.h"
#include "smp/Lock.h"

#include "core/app.h"


using namespace applet;


////////////////////////////////////////////////////////////////////////////////
// applet::EventCondition member definitions

EventCondition::EventCondition()
{
	SDL_AddEventWatch(OnEvent, this);
}

EventCondition::~EventCondition()
{
	SDL_DelEventWatch(OnEvent, this);
	smp::Yield();	// fingers crossed!
}

bool EventCondition::PopEvent(SDL_Event & event)
{
	smp::Lock<smp::SimpleMutex> lock(_mutex);
	if (_events.empty())
	{
		return false;
	}
	
	event = _events.back();
	_events.pop_back();
	return true;
}

bool EventCondition::Filter(SDL_Event const & event) const
{
	return true;
}

bool EventCondition::operator() (bool hurry)
{
	smp::Lock<smp::SimpleMutex> lock(_mutex);

	return hurry | (! _events.empty());
}

int EventCondition::OnEvent(void *userdata, SDL_Event * event)
{
	EventCondition & event_condition = ref(reinterpret_cast<EventCondition *>(userdata));
	
	if (event_condition.Filter(* event))
	{
		smp::Lock<smp::SimpleMutex> lock(event_condition._mutex);
		event_condition._events.push_back(* event);
	}
	
	return 0;
}

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

#include "ScriptThread.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::EventCondition member definitions

bool EventCondition::operator() (ScriptThread & script_thread)
{
	script_thread.GetEvent(_event);
	return _event.type != 0;
}

SDL_Event const & EventCondition::GetEvent() const
{
	ASSERT(_event.type != 0);
	return _event;
}

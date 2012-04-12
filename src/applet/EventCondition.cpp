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

#include "core/app.h"


using namespace applet;


////////////////////////////////////////////////////////////////////////////////
// applet::EventCondition member definitions

EventCondition::EventCondition()
{
	_event.type = 0;
}

bool EventCondition::operator() (bool hurry)
{
	ASSERT(_event.type == 0);
	
	if (app::PopEvent(_event))
	{
		return true;
	}
	
	if (hurry)
	{
		_event.type = SDL_QUIT;
		return true;
	}
	
	return false;
}

SDL_Event const & EventCondition::GetEvent() const
{
	ASSERT(_event.type != 0);
	return _event;
}

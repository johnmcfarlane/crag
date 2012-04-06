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


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::EventCondition member definitions

EventCondition::EventCondition()
{
	_event.type = 0;
}

bool EventCondition::operator() (/*Engine & engine*/)
{
	ASSERT(_event.type == 0);
	return app::PopEvent(_event);
}

SDL_Event const & EventCondition::GetEvent() const
{
	ASSERT(_event.type != 0);
	return _event;
}

//
//  TimeCondition.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-17.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "TimeCondition.h"

#include "Engine.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::TimeCondition member definitions

TimeCondition::TimeCondition(Time wake_position)
: _wake_position(wake_position)
{
}

bool TimeCondition::operator() (script::Engine & script_thread)
{
	return script_thread.GetTime() >= _wake_position;
}

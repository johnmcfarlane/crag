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

#include "core/app.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::TimeCondition member definitions

TimeCondition::TimeCondition(Time duration)
: _wake_position(app::GetTime() + duration)
{
}

bool TimeCondition::operator() ()
{
	return app::GetTime() >= _wake_position;
}

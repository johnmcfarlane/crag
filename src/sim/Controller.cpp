//
//  Controller.cpp
//  crag
//
//  Created by John on 2013/02/01.
//  Copyright 2009-2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Controller.h"

////////////////////////////////////////////////////////////////////////////////
// sim::Controller member definitions

using namespace sim;

Controller::Controller(Entity & entity)
: _entity(entity)
{
}

Controller::~Controller() 
{ 
}

#if defined(VERIFY)
void Controller::Verify() const
{
}
#endif

Entity & Controller::GetEntity()
{
	return _entity;
}

Entity const & Controller::GetEntity() const
{
	return _entity;
}

//
//  EntityHandle.cpp
//  crag
//
//  Created by John McFarlane on 2/12/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "EntityHandle.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// EntityHandleBase member definitions

// Tells simulation to destroy the object.
void EntityHandleBase::Destroy()
{
	// set message
	sim::Daemon::Call(_uid, & sim::Simulation::OnRemoveEntity);
	_uid = sim::Uid::null;
}

// Returns the UID of the entity being handled.
sim::Uid EntityHandleBase::GetUid() const
{
	return _uid;
}

// Sets the UID of the entity being handled.
void EntityHandleBase::SetUid(sim::Uid uid)
{
	_uid = uid;
}

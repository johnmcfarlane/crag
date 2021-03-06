//
//  Fiber.cpp
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Fiber.h"

using namespace ipc;


////////////////////////////////////////////////////////////////////////////////
// Platform-independent ipc::Fiber member definitions

bool Fiber::IsRunning() const
{
	CRAG_VERIFY(* this);
	return _is_running;
}

char const * Fiber::GetName() const
{
	CRAG_VERIFY(* this);
	return _name;
}

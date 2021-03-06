//
//  ListenerInterface.cpp
//  crag
//
//  Created by John on 2013-05-27.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ListenerInterface.h"

using namespace ipc;

////////////////////////////////////////////////////////////////////////////////
// ipc::ListenerBase member definitions

bool ListenerBase::CanExit()
{
	ASSERT(_counter >= 0);
	
	return _counter == 0;
}

int ipc::ListenerBase::_counter = 0;
ipc::ListenerBase::Mutex ipc::ListenerBase::_mutex;

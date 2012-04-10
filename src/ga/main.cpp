//
//  main.cpp
//  crag
//
//  Created by John McFarlane on 2012-04-07.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "main.h"

#include "applet/AppletInterface.h"
#include "applet/Engine.h"


using namespace ga;

void ga::main(applet::AppletInterface & applet_interface)
{
	DEBUG_MESSAGE("Meh!");
	applet::Daemon::Call(& applet::Engine::OnQuit);
}

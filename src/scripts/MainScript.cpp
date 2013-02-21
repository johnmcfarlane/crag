//
//  MainScript.cpp
//  crag
//
//  Created by John McFarlane on 2013-02-15.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MainScript.h"
#include "TestScript.h"

namespace
{
	void RunGa(applet::AppletInterface & applet_interface)
	{
	}
}

void MainScript(applet::AppletInterface & applet_interface)
{
#if 1
	Test(applet_interface);
#else
	RunGa(applet_interface);
#endif
}

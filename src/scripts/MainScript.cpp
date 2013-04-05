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

//#define GA

namespace
{
#if defined(GA)
	void RunGa(applet::AppletInterface & applet_interface)
	{
	}
#endif
}

void MainScript(applet::AppletInterface & applet_interface)
{
#if defined(GA)
	RunGa(applet_interface);
#else
	Test(applet_interface);
#endif
}

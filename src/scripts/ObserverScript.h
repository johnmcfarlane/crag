//
//  ObserverScript.h
//  crag
//
//  Created by John McFarlane on 2012-02-11.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace applet
{
	class AppletInterface;
}

namespace sim 
{ 
	DECLARE_CLASS_HANDLE(Observer);	// sim::ObserverHandle
}

void ObserverScript(applet::AppletInterface & applet_interface, sim::ObserverHandle observer);

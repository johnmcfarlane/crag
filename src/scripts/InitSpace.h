//
//  InitSpace.h
//  crag
//
//  Created by John McFarlane on 2014-07-18.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

namespace geom
{
	class Space;
}

namespace applet
{
	class AppletInterface;
}

// ensures Lodding system kicks off; 
// call after spawning formations and before a brief pause;
// then spawn skybox and physical objects
void InitSpace(applet::AppletInterface & applet_interface, geom::Space const & space);

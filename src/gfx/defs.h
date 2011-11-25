//
//  defs.h
//  crag
//
//  Created by John McFarlane on 7/18/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/Uid.h"


namespace gfx
{
	// unique identifier for gfx::Object;
	// used when refering to objects from outside the render thread
	typedef smp::Uid Uid;
	
	// base class of objects which are rendered
	class Object;
	
	// storage type for Object
	typedef std::map<Uid, Object *> ObjectMap;
}

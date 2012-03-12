//
//  Script.h
//  crag
//
//  Created by John McFarlane on 2012-03-07.
//  Copyright (c) 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp/ObjectBase.h"

namespace script
{
	// forward-declarations
	class FiberInterface;
	class ScriptThread;
	
	// Base class for scripts, which are run in fibers.
	class Script : public smp::ObjectBase<Script, ScriptThread>
	{
	public:
		virtual ~Script() { }
		virtual void operator() (FiberInterface & fiber) = 0;
	};
}

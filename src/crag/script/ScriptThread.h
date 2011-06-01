/*
 *  ScriptThread.h
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "smp/Actor.h"
#include "core/Singleton.h"


namespace script
{
	
	// The scripting support is centered here.
	// When Run finished, the program is done.
	class ScriptThread : public smp::Actor<ScriptThread>
	{
		OBJECT_SINGLETON(ScriptThread);
	public:
		ScriptThread();
		~ScriptThread();
		
		virtual void Run();
		
	private:
		void Run(char const * source_filename);
	};
	
}

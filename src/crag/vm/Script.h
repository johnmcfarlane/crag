/*
 *  Script.h
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009 - 2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include <v8.h>


namespace vm
{
	
	class Script
	{
	public:
		Script();
		
		void Compile(char const * source_code);
		void Compile(std::istream & source_file);

		void Run();
		
	private:
		v8::Handle<v8::Script> script;
	};

}

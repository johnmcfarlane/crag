/*
 *  vm.h
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
	
	class Singleton
	{
	public:
		Singleton();
		~Singleton();
		
	private:
		v8::HandleScope handle_scope;
		v8::Persistent<v8::Context> context;
		v8::Context::Scope context_scope;
	};
	
}

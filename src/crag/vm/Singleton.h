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

#include "core/Singleton.h"

#include <v8.h>


namespace vm
{
	
	// A collection of the v8-related objects that are unique in the simulation (so far).
	class Singleton : public core::Singleton<Singleton>
	{
		friend class Scope;
		
	public:
		Singleton();
		~Singleton();
		
		void SetAccessor(v8::Handle<v8::String> name,
						 v8::AccessorGetter getter,
						 v8::AccessorSetter setter = 0,
						 v8::Handle<v8::Value> data = v8::Handle<v8::Value>(),
						 v8::AccessControl settings = v8::DEFAULT,
						 v8::PropertyAttribute attribute = v8::None);
		
		void Begin();
		
	private:
		v8::HandleScope handle_scope;
		v8::Handle<v8::ObjectTemplate> global;
		v8::Persistent<v8::Context> context;
	};
	
}

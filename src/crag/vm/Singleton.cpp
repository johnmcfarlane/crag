/*
 *  Singleton.cpp
 *  crag
 *
 *  Created by John McFarlane on 1/19/11.
 *  Copyright 2009 - 2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Singleton.h"


vm::Singleton::Singleton()
: global(v8::ObjectTemplate::New())
{
}

vm::Singleton::~Singleton()
{
	context.Dispose();
}

void vm::Singleton::SetAccessor(v8::Handle<v8::String> name,
				 v8::AccessorGetter getter,
				 v8::AccessorSetter setter,
				 v8::Handle<v8::Value> data,
				 v8::AccessControl settings,
				 v8::PropertyAttribute attribute)
{
	global->SetAccessor(name, getter, setter, data, settings, attribute);
}

void vm::Singleton::Begin()
{
	context = v8::Context::New(NULL, global);	
}

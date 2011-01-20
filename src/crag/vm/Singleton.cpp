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
: context(v8::Context::New())
, context_scope(context)
{
}


vm::Singleton::~Singleton()
{
	context.Dispose();
}

/*
 *  Scope.cpp
 *  crag
 *
 *  Created by John McFarlane on 1/25/11.
 *  Copyright 2009 - 2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Scope.h"

vm::Scope::Scope()
: context_scope(vm::Singleton::Get().context)
{
}

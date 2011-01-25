/*
 *  Scope.h
 *  crag
 *
 *  Created by John McFarlane on 1/25/11.
 *  Copyright 2009 - 2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Singleton.h"


namespace vm
{

	// Precede scripting code with an instance of Scope.
	// This provides v8 with the scoping information it requires.
	class Scope
	{
	public:
		Scope();
		
	private:
		v8::Context::Scope context_scope;
	};

}

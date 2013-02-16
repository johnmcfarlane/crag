//
//  RootNode.h
//  crag
//
//  Created by John McFarlane on 2011/11/21.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Object.h"


namespace gfx { DECLARE_CLASS_HANDLE(Object); }	// gfx::ObjectHandle


namespace gfx
{
	// An object which contains sub-objects
	class RootNode : public Object
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef Object super;
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		RootNode(Init const & init, Transformation const & transformation);
	};
}

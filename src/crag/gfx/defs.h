//
//  defs.h
//  crag
//
//  Created by John McFarlane on 7/18/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace gfx
{
	// categorization of objects drawn at rendered layers
	namespace Layer
	{
		enum type
		{
			pre_render,
			background,
			light,
			foreground,
			num,
			
			begin = 0,
			end = num
		};
	}
	
	// base class of objects which are rendered
	class Object;
	
	// storage type for Object
	typedef std::set<Object *> ObjectSet;
	
	// Passed to gfx:Renderer; specialized for classes derived from gfx::Object;
	template <typename OBJECT> class UpdateObjectMessage;
}

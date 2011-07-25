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
	// categorization of objects drawn at rendered stages
	namespace RenderStage
	{
		enum type
		{
			background,
			light,
			foreground,
			num
		};
	}
	
	// base class of objects which are rendered
	class Object;

	// storage type for Object
	typedef std::vector<Object const *> ObjectVector;

	// Passed to gfx:Renderer; specialized for classes derived from gfx::Object;
	template <typename OBJECT> struct UpdateObjectMessage;
}

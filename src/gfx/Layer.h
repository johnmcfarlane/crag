//
//  Layer.h
//  crag
//
//  Created by John McFarlane on 2011-11-22.
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
			background,
			light,
			foreground,
			num,
			
			begin = 0,
			end = num,
			none = num
		};
		
		inline type operator ++ (type & layer_index)
		{
			return layer_index = type(int(layer_index) + 1);
		}
		
#if defined(VERIFY)
		void Verify(type layer);
#else
		inline void Verify(type layer) { }
#endif
	}
}

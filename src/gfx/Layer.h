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
			pre_render,
			background,
			light,
			foreground,
			num,
			
			begin = 0,
			end = num
		};
		
		namespace Map
		{
			// bitmap of bools per layer
			enum type
			{
				none = 0,
				all = (1 << num) - 1
			};
		}
		
		Map::type ToMap(type layer);
		
		inline type operator ++ (type & layer_index)
		{
			return layer_index = type(int(layer_index) + 1);
		}
		
#if defined(VERIFY)
		void Verify(type layer);
		void Verify(Map::type map);
#else
		inline void Verify(type layer) { }
		inline void Verify(Map::type map) { }
#endif
	}
	
	inline Layer::Map::type operator |= (Layer::Map::type & lhs, Layer::Map::type const & rhs)
	{
		lhs = static_cast<Layer::Map::type>(static_cast<int>(lhs) | static_cast<int>(rhs));
		return lhs;
	}
	
	inline Layer::Map::type operator |= (Layer::Map::type & lhs, Layer::type const & rhs)
	{
		return lhs |= Layer::ToMap(rhs);
	}
	
	inline Layer::Map::type operator | (Layer::Map::type const & lhs, Layer::Map::type const & rhs)
	{
		Layer::Map::type result = static_cast<Layer::Map::type>(static_cast<int>(lhs) | static_cast<int>(rhs));
		return result;
	}
	
	inline Layer::Map::type operator | (Layer::type const & lhs, Layer::type const & rhs)
	{
		Layer::Map::type result = ToMap(lhs) | ToMap(rhs);
		return result;
	}
}

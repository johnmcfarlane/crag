/*
` *  IndexBuffer.h
 *  Crag
 *
 *  Created by john on 5/3/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */
 
#pragma once

#include "glpp/glpp.h"

#include <vector>


namespace gfx
{
	
	class IndexBuffer : private std::vector<GLuint>
	{
	public:
		IndexBuffer(int capacity);

		// number of indices
		int GetSize() const 
		{ 
			return size(); 
		}
		
		int GetCapacity() const
		{
			return capacity();
		}

		// number of spare faces
		int GetSlack() const 
		{ 
			return capacity() - size(); 
		}
		
		void Clear();
		
		void PushBack(value_type index)
		{
			push_back(index);
		}
		
		value_type const * GetArray() const;
	};

}

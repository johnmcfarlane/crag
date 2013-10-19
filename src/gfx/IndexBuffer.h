//
//  IndexBuffer.h
//  crag
//
//  Created by john on 5/3/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//
 
#pragma once

#include "glHelpers.h"

#include "smp/vector.h"

namespace gfx
{
	
	class IndexBuffer : private smp::vector<ElementIndex>
	{
		typedef smp::vector<ElementIndex> Super;

	public:
		IndexBuffer(int capacity);

		// number of indices
		int GetSize() const 
		{ 
			return static_cast<int>(size());
		}
		
		int GetCapacity() const
		{
			return static_cast<int>(capacity());
		}

		// number of spare faces
		int GetSlack() const 
		{ 
			return GetCapacity() - GetSize();
		}
		
		void Clear();
		
		value_type & PushBack()
		{
			return * grow(3);
		}
		
		Super::const_iterator begin() const
		{
			return Super::begin();
		}
		
		Super::const_iterator end() const
		{
			return Super::end();
		}
		
		typedef Super::value_type value_type;
		value_type const * GetArray() const;
	};

}

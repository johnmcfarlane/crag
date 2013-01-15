//
//  PointBuffer.h
//  crag
//
//  Created by John on 5/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Point.h"

namespace form 
{
	class PointBuffer : private core::object_pool<Point>
	{
		typedef core::object_pool<Point> super;
	public:
		PointBuffer(int max_num_verts);
		
		void Clear();
		void FastClear();
		
		Point * Create()
		{
			return super::create();
		}
		
		void Destroy(Point * ptr)
		{
			super::destroy(ptr);
		}
		
#if defined(VERIFY)
		void Verify() const;
#endif
	};
}

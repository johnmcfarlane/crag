/*
 *  PointBuffer.h
 *  crag
 *
 *  Created by John on 5/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Point.h"

#include "core/Pool.h"


//#define POINT_BUFFER_MT_SAFE

#if defined(POINT_BUFFER_MT_SAFE)
#include "smp/Lock.h"
#define POINT_BUFFER_LOCK smp::Lock lock(mutex)
#else
#define POINT_BUFFER_LOCK DO_NOTHING
#endif


namespace form 
{
	class PointBuffer : private Pool<Point>
	{
		typedef Pool<Point> super;
	public:
		PointBuffer(int max_num_verts);
		
		void Clear();
		void FastClear();
		
		Point * Alloc()
		{
			POINT_BUFFER_LOCK;
			return super::Alloc();
		}
		
		void Free(Point * ptr)
		{
			POINT_BUFFER_LOCK;
			super::Free(ptr);
		}
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
	private:
#if defined(POINT_BUFFER_MT_SAFE)
		mutable smp::Mutex mutex;
#endif
	};
}

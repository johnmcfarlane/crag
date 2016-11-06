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

#include "core/object_pool.h"

namespace form 
{
	class PointBuffer
	{
		typedef core::object_pool<Point> Pool;
	public:
		PointBuffer(int max_num_verts);
		
		bool IsEmpty() const;
		void ClearPointers();
		
		Point * Create();
		void Destroy(Point * ptr);
		
#if defined(CRAG_VERIFY_ENABLED)
		void VerifyAllocatedElement(Point const & element) const;
		CRAG_VERIFY_INVARIANTS_DECLARE(PointBuffer);
#endif

private:
		Pool _pool;
	};
}

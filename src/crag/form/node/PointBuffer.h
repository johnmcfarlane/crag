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


namespace form 
{
	class PointBuffer : public Pool<Point>
	{
	public:
		PointBuffer(int max_num_verts);
		
		void Clear();

		//DUMP_OPERATOR_DECLARATION(VertexBuffer);
	};
}

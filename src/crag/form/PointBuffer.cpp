/*
 *  PointBuffer.cpp
 *  crag
 *
 *  Created by John on 5/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "PointBuffer.h"

form::PointBuffer::PointBuffer(int max_num_verts) 
: Pool<Point>(max_num_verts)
{
}

void form::PointBuffer::Clear()
{
	Point * begin = GetArray();
	Point const * const end = begin + GetMaxUsed();
	for (Point * i = begin; i != end; ++ i)
	{
		Point & point = * i;
		point.vert = nullptr;
	}
}

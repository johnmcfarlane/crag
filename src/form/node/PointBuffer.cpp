/*
 *  PointBuffer.cpp
 *  crag
 *
 *  Created by John on 5/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "PointBuffer.h"

form::PointBuffer::PointBuffer(int max_num_verts) 
: Pool<Point>(max_num_verts)
{
}

void form::PointBuffer::Clear()
{
	POINT_BUFFER_LOCK;
	
	Point * begin = GetArray();
	Point const * const used_end = begin + GetMaxUsed();
	for (Point * i = begin; i != used_end; ++ i)
	{
		Point & point = * i;
		point.vert = nullptr;
	}
}

void form::PointBuffer::FastClear()
{
	POINT_BUFFER_LOCK;
	super::FastClear();
}

#if defined(VERIFY)
void form::PointBuffer::Verify() const
{
	POINT_BUFFER_LOCK;
	super::Verify();
}
#endif

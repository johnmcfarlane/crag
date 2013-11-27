//
//  PointBuffer.cpp
//  crag
//
//  Created by John on 5/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "PointBuffer.h"

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// PointBuffer member definitions

PointBuffer::PointBuffer(std::size_t max_num_verts) 
: _pool(max_num_verts)
{
}

bool PointBuffer::IsEmpty() const
{
	return _pool.empty();
}

void form::PointBuffer::ClearPointers()
{
	_pool.for_each_activated([] (Point & point)
	{
		point.vert = nullptr;
	});
}

Point * PointBuffer::Create()
{
	auto creation = _pool.create();
	return creation;
}

void PointBuffer::Destroy(Point * ptr)
{
	_pool.destroy(ptr);
}

#if defined(CRAG_VERIFY_ENABLED)
void PointBuffer::VerifyAllocatedElement(Point const & element) const
{
	_pool.VerifyAllocatedElement(element);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(PointBuffer, object)
	CRAG_VERIFY(object._pool);
CRAG_VERIFY_INVARIANTS_DEFINE_END
#endif

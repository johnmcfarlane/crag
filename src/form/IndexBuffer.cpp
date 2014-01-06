//
//  IndexBuffer.cpp
//  crag
//
//  Created by john on 5/3/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "IndexBuffer.h"

#include "core/memory.h"

using namespace form;

//////////////////////////////////////////////////////////////////////
// IndexBuffer member definitions

IndexBuffer::IndexBuffer(int buffer_capacity)
: super(buffer_capacity)
{
}

void IndexBuffer::Clear()
{
	clear();
}

IndexBuffer::value_type const * IndexBuffer::GetArray() const
{
	return & front();
}

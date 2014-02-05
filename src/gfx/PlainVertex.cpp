//
//  gfx/PlainVertex.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "PlainVertex.h"

#include "VertexBufferObject.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::PlainVertex helper functions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(PlainVertex, self)
	CRAG_VERIFY(self.pos);
CRAG_VERIFY_INVARIANTS_DEFINE_END

////////////////////////////////////////////////////////////////////////////////
// gfx::PlainVertex GL state helper functions

template <>
void EnableClientState<PlainVertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
}

template <>
void DisableClientState<PlainVertex>()
{
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<PlainVertex>()
{
	gfx::VertexAttribPointer<1, PlainVertex, geom::Vector<float, 3>, & PlainVertex::pos>();
}


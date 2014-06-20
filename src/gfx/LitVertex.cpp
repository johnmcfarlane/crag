//
//  LitVertex.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "LitVertex.h"

#include "VertexBufferObject.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::LitVertex helper functions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(LitVertex, self)
	CRAG_VERIFY(self.pos);
	CRAG_VERIFY(self.norm);
CRAG_VERIFY_INVARIANTS_DEFINE_END

////////////////////////////////////////////////////////////////////////////////
// gfx::LitVertex GL state helper functions

template <>
void EnableClientState<LitVertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glEnableVertexAttribArray(3));
}

template <>
void DisableClientState<LitVertex>()
{
	GL_CALL(glDisableVertexAttribArray(3));
	GL_CALL(glDisableVertexAttribArray(2));
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<LitVertex>()
{
	gfx::VertexAttribPointer<1, LitVertex, geom::Vector<float, 3>, & LitVertex::pos>();
	gfx::VertexAttribPointer<2, LitVertex, geom::Vector<float, 3>, & LitVertex::norm>();
	gfx::VertexAttribPointer<3, LitVertex, gfx::Color4b, & LitVertex::color>();
}


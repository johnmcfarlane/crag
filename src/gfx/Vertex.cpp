//
//  Vertex.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-23.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Vertex.h"

#include "VertexBufferObject.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Vertex helper functions

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Vertex, self)
	CRAG_VERIFY(self.pos);
	CRAG_VERIFY(self.norm);
CRAG_VERIFY_INVARIANTS_DEFINE_END

////////////////////////////////////////////////////////////////////////////////
// gfx::Vector3 GL state helper functions

template <>
void EnableClientState<Vector3>()
{
	GL_CALL(glEnableVertexAttribArray(1));
}

template <>
void DisableClientState<Vector3>()
{
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<Vector3>()
{
	GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), nullptr));
}

////////////////////////////////////////////////////////////////////////////////
// gfx::Vertex GL state helper functions

template <>
void EnableClientState<Vertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glEnableVertexAttribArray(3));
}

template <>
void DisableClientState<Vertex>()
{
	GL_CALL(glDisableVertexAttribArray(3));
	GL_CALL(glDisableVertexAttribArray(2));
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<Vertex>()
{
	gfx::VertexAttribPointer<1, Vertex, geom::Vector<float, 3>, & Vertex::pos>();
	gfx::VertexAttribPointer<2, Vertex, geom::Vector<float, 3>, & Vertex::norm>();
	gfx::VertexAttribPointer<3, Vertex, gfx::Color4b, & Vertex::color>();
}


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

#include "gfx/VertexBufferObject.h"


////////////////////////////////////////////////////////////////////////////////
// form::Vertex helper functions

template <>
void EnableClientState<form::Vertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
	GL_CALL(glEnableVertexAttribArray(2));
	GL_CALL(glEnableVertexAttribArray(3));
}

template <>
void DisableClientState<form::Vertex>()
{
	GL_CALL(glDisableVertexAttribArray(3));
	GL_CALL(glDisableVertexAttribArray(2));
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<form::Vertex>()
{
	gfx::VertexAttribPointer<1, form::Vertex, geom::Vector<float, 3>, & form::Vertex::pos>();
	gfx::VertexAttribPointer<2, form::Vertex, geom::Vector<float, 3>, & form::Vertex::norm>();
	gfx::VertexAttribPointer<3, form::Vertex, gfx::Color4b, & form::Vertex::col>();
}


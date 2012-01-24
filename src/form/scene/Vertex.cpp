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
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

template <>
void DisableClientState<form::Vertex>()
{
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}

template <>
void Pointer<form::Vertex>()
{
	gfx::VertexPointer<form::Vertex, 3, & form::Vertex::pos>();
	gfx::NormalPointer<form::Vertex, & form::Vertex::norm>();
	gfx::ColorPointer<form::Vertex, & form::Vertex::col>();
}


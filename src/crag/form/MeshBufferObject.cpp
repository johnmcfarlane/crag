/*
 *  form/MeshBufferObject.cpp
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "form/MeshBufferObject.h"
#include "form/Mesh.h"

#include "core/ConfigEntry.h"

#include "gfx/Color.h"


form::MeshBufferObject::MeshBufferObject()
: max_index(0)
{
}

void form::MeshBufferObject::Set(form::Mesh const & mesh)
{
	SetVbo(* mesh.GetVertices());
	SetIbo(mesh.GetIndices());
}

void form::MeshBufferObject::BeginDraw(bool color)
{
	// Enable all the magic buffer thingeys.
	glEnableClientState( GL_VERTEX_ARRAY );
	if (color) {
		glEnableClientState( GL_NORMAL_ARRAY );
		glEnableClientState( GL_COLOR_ARRAY );
	}
	
	Bind();
	
	const Vertex * null_vert = 0;
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), & null_vert->pos);
	if (color) {
		glNormalPointer(GL_FLOAT, sizeof(Vertex), & null_vert->norm);
		glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(Vertex), & null_vert->red);
	}
}

void form::MeshBufferObject::EndDraw()
{
	// Disable all the magic buffer thingeys.
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY );
}	

void form::MeshBufferObject::Draw()
{
	//Assert(ibo.IsBound());	// sometimes fails for personal amusement
	gl::Mesh<Vertex>::Draw(0, max_index);
}

void form::MeshBufferObject::SetVbo(form::VertexBuffer const & vertices)
{
	gl::Mesh<Vertex>::SetVbo(vertices.GetMaxUsed(), vertices.GetArray());
}

void form::MeshBufferObject::SetIbo(IndexBuffer const & indices)
{
	max_index = indices.GetSize();
	gl::Mesh<Vertex>::SetIbo(max_index, indices.GetArray());
}

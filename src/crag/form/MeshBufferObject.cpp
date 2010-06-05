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
	SetVbo(mesh.GetVertices());
	SetIbo(mesh.GetIndices());
}

void form::MeshBufferObject::BeginDraw(bool color)
{
	// Enable all the magic buffer thingeys.
	glEnableClientState( GL_VERTEX_ARRAY );
	if (color) {
		glEnableClientState( GL_NORMAL_ARRAY );
#if defined(FORM_VERTEX_COLOR)
		glEnableClientState( GL_COLOR_ARRAY );
#endif
	}
#if defined(FORM_VERTEX_TEXTURE)
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
	
	Bind();
	
	const Vertex * null_vert = 0;
	glVertexPointer(3, GL_FLOAT, sizeof(Vertex), & null_vert->pos);
	if (color) {
		glNormalPointer(GL_FLOAT, sizeof(Vertex), & null_vert->norm);
#if defined(FORM_VERTEX_COLOR)
		glColorPointer(3, GL_UNSIGNED_BYTE, sizeof(Vertex), & null_vert->red);
#endif
	}
#if defined(FORM_VERTEX_TEXTURE)
	glTexCoordPointer (2, GL_FLOAT, sizeof(Vertex), & null_vert->texture);
#endif
}

void form::MeshBufferObject::EndDraw()
{
	// Disable all the magic buffer thingeys.
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
#if defined(FORM_VERTEX_COLOR)
	glDisableClientState(GL_COLOR_ARRAY );
#endif
#if defined(FORM_VERTEX_TEXTURE)
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
}	

void form::MeshBufferObject::Draw()
{
	//Assert(ibo.IsBound());	// sometimes fails for personal amusement
	gl::Mesh<Vertex>::Draw(0, max_index);
}

void form::MeshBufferObject::SetVbo(VertexBuffer const & vertices)
{
	size_t num_vertices = vertices.Size();
	Vertex const * vertex_array = (num_vertices > 0) ? vertices.GetArray() : nullptr;
	gl::Mesh<Vertex>::SetVbo(num_vertices, vertex_array);
}

void form::MeshBufferObject::SetIbo(gfx::IndexBuffer const & indices)
{
	size_t num_indices = indices.GetSize();
	gfx::IndexBuffer::value_type const * index_array = (num_indices > 0) ? indices.GetArray() : nullptr;
	gl::Mesh<Vertex>::SetIbo(num_indices, index_array);

	max_index = num_indices;
}

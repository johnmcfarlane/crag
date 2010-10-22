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

#include "MeshBufferObject.h"
#include "Mesh.h"

#include "core/ConfigEntry.h"

#include "gfx/Color.h"
#include "gfx/Pov.h"


form::MeshBufferObject::MeshBufferObject()
: max_index(0)
, properties(false)
{
}

void form::MeshBufferObject::Set(form::Mesh const & mesh)
{
	SetVbo(mesh.GetVertices());
	SetIbo(mesh.GetIndices());
	
	properties = mesh.GetProperties();
}

int form::MeshBufferObject::GetNumPolys() const
{
	return max_index / 3;
}

void form::MeshBufferObject::BeginDraw(gfx::Pov pov, bool color) const
{
	Assert (max_index > 0);
	
	// Adjust our copy of the pov for mesh's origin and set as matrix.
	pov.pos -= properties.origin;
	GLPP_CALL(glMatrixMode(GL_MODELVIEW));
	gl::LoadMatrix(pov.CalcModelViewMatrix().GetArray());
	
	if (properties.flat_shaded) 
	{
		GLPP_CALL(glShadeModel(GL_FLAT));
	}
	
	// Enable all the magic buffer thingeys.
	GLPP_CALL(glEnableClientState( GL_VERTEX_ARRAY ));
	if (color) {
		GLPP_CALL(glEnableClientState( GL_NORMAL_ARRAY ));
#if defined(FORM_VERTEX_COLOR)
		GLPP_CALL(glEnableClientState( GL_COLOR_ARRAY ));
#endif
	}
#if defined(FORM_VERTEX_TEXTURE)
	GLPP_CALL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
#endif
	
	Bind();
	
	const Vertex * null_vert = 0;
	GLPP_CALL(glVertexPointer(3, GL_FLOAT, sizeof(Vertex), & null_vert->pos));
	if (color) {
		GLPP_CALL(glNormalPointer(GL_FLOAT, sizeof(Vertex), & null_vert->norm));
#if defined(FORM_VERTEX_COLOR)
		GLPP_CALL(glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(Vertex), & null_vert->color));
#endif
	}
#if defined(FORM_VERTEX_TEXTURE)
	GLPP_CALL(glTexCoordPointer (2, GL_FLOAT, sizeof(Vertex), & null_vert->texture));
#endif
}

void form::MeshBufferObject::EndDraw() const
{
	Assert (max_index > 0);
		
	// Disable all the magic buffer thingeys.
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
#if defined(FORM_VERTEX_COLOR)
	glDisableClientState(GL_COLOR_ARRAY );
#endif
#if defined(FORM_VERTEX_TEXTURE)
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif

	if (properties.flat_shaded) 
	{
		GLPP_CALL(glShadeModel(GL_SMOOTH));
	}
}

void form::MeshBufferObject::Draw() const
{
	Assert (max_index > 0);	
	//Assert(ibo.IsBound());	// sometimes fails for personal amusement
	GLPP_CALL(gl::Mesh<Vertex>::Draw(0, max_index));
}

void form::MeshBufferObject::SetVbo(VertexBuffer const & vertices)
{
	size_t num_vertices = vertices.Size();
	Vertex const * vertex_array = (num_vertices > 0) ? vertices.GetArray() : nullptr;
	gl::Mesh<Vertex>::SetVbo(num_vertices, vertex_array);
}

void form::MeshBufferObject::SetIbo(gfx::IndexBuffer const & indices)
{
	max_index = indices.GetSize();
	gfx::IndexBuffer::value_type const * index_array = (max_index > 0) ? indices.GetArray() : nullptr;
	gl::Mesh<Vertex>::SetIbo(max_index, index_array);
}

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

// TODO: http://www.opengl.org/wiki/Vertex_Specification_Best_Practices
// TODO: Section "Dynamic VBO" - test the glBufferData(NULL) thing once and for all.
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

void form::MeshBufferObject::Activate(gfx::Pov pov, bool color) const
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

	super::Bind();
	super::Activate();
}

void form::MeshBufferObject::Deactivate() const
{
	Assert (max_index > 0);

	super::Deactivate();
	super::Unbind();
	
	if (properties.flat_shaded) 
	{
		GLPP_CALL(glShadeModel(GL_SMOOTH));
	}
}

void form::MeshBufferObject::Draw() const
{
	Assert (max_index > 0);	
	//Assert(ibo.IsBound());	// sometimes fails for personal amusement
	GLPP_CALL(super::Draw(0, max_index));
}

void form::MeshBufferObject::SetVbo(VertexBuffer const & vertices)
{
	size_t num_vertices = vertices.Size();
	Vertex const * vertex_array = (num_vertices > 0) ? vertices.GetArray() : nullptr;
	super::SetVbo(num_vertices, vertex_array);
}

void form::MeshBufferObject::SetIbo(gfx::IndexBuffer const & indices)
{
	max_index = indices.GetSize();
	gfx::IndexBuffer::value_type const * index_array = (max_index > 0) ? indices.GetArray() : nullptr;
	super::SetIbo(max_index, index_array);
}

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

form::MeshProperties::Vector3 const & form::MeshBufferObject::GetOrigin() const
{
	return properties._origin;
}

void form::MeshBufferObject::Activate(gfx::Pov pov) const
{
	Assert (max_index > 0);
	
	pov.SetModelView(properties._origin);
	
	if (properties._flat_shaded) 
	{
		gl::ShadeModel(GL_FLAT);
	}

	super::Activate();
}

void form::MeshBufferObject::Deactivate() const
{
	Assert (max_index > 0);

	super::Deactivate();
	super::Unbind();
	
	if (properties._flat_shaded) 
	{
		gl::ShadeModel(GL_SMOOTH);
	}
}

void form::MeshBufferObject::Draw() const
{
	super::Draw(GL_TRIANGLES, max_index);
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

//
//  MeshBufferObject.cpp
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MeshBufferObject.h"
#include "Mesh.h"

#include "core/ConfigEntry.h"

#include "gfx/Color.h"
#include "gfx/Pov.h"


////////////////////////////////////////////////////////////////////////////////
// MeshBufferObject member definitions

#if defined(VERIFY)
void form::MeshBufferObject::Verify() const
{
	super::Verify();
	VerifyOp(max_index, >=, 0);
	VerifyObject(properties);
}
#endif

form::MeshBufferObject::MeshBufferObject()
: max_index(0)
{
}

form::MeshBufferObject::~MeshBufferObject()
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

form::MeshProperties::Vector const & form::MeshBufferObject::GetOrigin() const
{
	return properties._origin;
}

void form::MeshBufferObject::Activate() const
{
	super::Bind();
	super::Activate();
}

void form::MeshBufferObject::Deactivate() const
{
	super::Deactivate();
	super::Unbind();
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

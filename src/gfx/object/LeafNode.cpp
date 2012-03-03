//
//  LeafNode.cpp
//  crag
//
//  Created by John McFarlane on 2011-11-23.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "LeafNode.h"


////////////////////////////////////////////////////////////////////////////////
// gfx::LeafNode member definitions

using namespace gfx;

LeafNode::LeafNode(Layer::type layer)
: Object(leaf)
, _model_view_transformation(Transformation::Matrix::Identity())
, _render_depth(0)
, _layer(layer)
, _program(nullptr)
, _mesh_resource(nullptr)
, _is_opaque(true)
{
}

#if defined(VERIFY)
void LeafNode::Verify() const
{
	super::Verify();
	
	VerifyObject(_model_view_transformation);
}
#endif

void LeafNode::SetModelViewTransformation(Transformation const & model_view_transformation)
{
	_model_view_transformation = model_view_transformation;
	
	Transformation::Matrix const & matrix = _model_view_transformation.GetMatrix();
	_render_depth = matrix[1][3];

	VerifyObject(* this);
}

LeafNode::Transformation const & LeafNode::GetModelViewTransformation() const
{
	return _model_view_transformation;
}

bool gfx::operator < (LeafNode const & lhs, LeafNode const & rhs)
{
	if (lhs._is_opaque)
	{
		if (rhs._is_opaque)
		{
			ptrdiff_t mesh_index_diff = reinterpret_cast<char const *>(lhs._mesh_resource) - reinterpret_cast<char const *>(rhs._mesh_resource);
			if (mesh_index_diff != 0)
			{
				return mesh_index_diff < 0;
			}
			
			ptrdiff_t shader_index_diff = reinterpret_cast<char const *>(lhs._program) - reinterpret_cast<char const *>(rhs._program);
			if (shader_index_diff != 0)
			{
				return shader_index_diff < 0;
			}
			
			return lhs._render_depth > rhs._render_depth;
		}

		return true;
	}
	else
	{
		if (rhs._is_opaque)
		{
			return false;
		}
		
		return lhs._render_depth < rhs._render_depth;
	}
}

Layer::type LeafNode::GetLayer() const
{
	return _layer;
}

Program const * LeafNode::GetProgram() const
{
	return _program;
}

void LeafNode::SetProgram(Program const * program)
{
	_program = program;
}

MeshResource const * LeafNode::GetMeshResource() const
{
	return _mesh_resource;
}

void LeafNode::SetMeshResource(MeshResource const * mesh_resource)
{
	_mesh_resource = mesh_resource;
}

bool LeafNode::IsOpaque() const
{
	return _is_opaque;
}

void LeafNode::SetIsOpaque(bool is_opaque)
{
	_is_opaque = is_opaque;
}

bool LeafNode::GetRenderRange(RenderRange & range) const 
{ 
	return false;
}

LeafNode::PreRenderResult LeafNode::PreRender(Renderer const & renderer)
{
	return ok;
}

void LeafNode::Render(Renderer const & renderer) const
{
	Assert(false);
}

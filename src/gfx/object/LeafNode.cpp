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

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::LeafNode member definitions

LeafNode::LeafNode(Init const & init, Transformation const & local_transformation, Layer layer, bool is_opaque, bool casts_shadow)
: Object(init, local_transformation)
, _model_view_transformation(Transformation::Matrix44::Identity())
, _render_depth(0)
, _layer(layer)
, _program(nullptr)
, _vbo_resource(nullptr)
, _is_opaque(is_opaque)
, _casts_shadow(casts_shadow)
{
	CRAG_VERIFY(* this);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(LeafNode, object)
	CRAG_VERIFY(static_cast<LeafNode::super const &>(object));
	
	CRAG_VERIFY(object._model_view_transformation);
CRAG_VERIFY_INVARIANTS_DEFINE_END

LeafNode & LeafNode::CastLeafNodeRef()
{
	return * this;
}

LeafNode const & LeafNode::CastLeafNodeRef() const
{
	return * this;
}

LeafNode * LeafNode::CastLeafNodePtr()
{
	return this;
}

LeafNode const * LeafNode::CastLeafNodePtr() const
{
	return this;
}

void LeafNode::UpdateModelViewTransformation(Transformation const & model_view_transformation)
{
	SetModelViewTransformation(model_view_transformation);
}

void LeafNode::SetModelViewTransformation(Transformation const & model_view_transformation)
{
	_model_view_transformation = model_view_transformation;
	
	Transformation::Matrix44 const & matrix = _model_view_transformation.GetMatrix();
	_render_depth = matrix[1][3];

	CRAG_VERIFY(* this);
}

Transformation const & LeafNode::GetModelViewTransformation() const
{
	return _model_view_transformation;
}

Transformation const & LeafNode::GetShadowModelViewTransformation() const
{
	return _model_view_transformation;
}

bool gfx::operator < (LeafNode const & lhs, LeafNode const & rhs)
{
	if (lhs._is_opaque)
	{
		if (rhs._is_opaque)
		{
			ptrdiff_t mesh_index_diff = reinterpret_cast<char const *>(lhs._vbo_resource) - reinterpret_cast<char const *>(rhs._vbo_resource);
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

Layer LeafNode::GetLayer() const
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

VboResource const * LeafNode::GetVboResource() const
{
	return _vbo_resource;
}

void LeafNode::SetVboResource(VboResource const * vbo_resource)
{
	_vbo_resource = vbo_resource;
}

bool LeafNode::IsOpaque() const
{
	return _is_opaque;
}

bool LeafNode::CastsShadow() const
{
	return _casts_shadow;
}

bool LeafNode::GetRenderRange(RenderRange &) const 
{ 
	return false;
}

LeafNode::PreRenderResult LeafNode::PreRender()
{
	return ok;
}

void LeafNode::GenerateShadowVolume(Light const &, ShadowVolume &) const
{
	ASSERT(false);
}

void LeafNode::Render(Engine const &) const
{
	ASSERT(false);
}

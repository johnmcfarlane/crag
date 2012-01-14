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

LeafNode::LeafNode(Layer::type layer, ProgramIndex::type shader_index)
: Object(leaf)
, _render_depth(0)
, _layer(layer)
, _program_index(shader_index)
, _is_opaque(true)
{
}

#if defined(VERIFY)
void LeafNode::Verify() const
{
	VerifyObject(_model_view_transformation);
}
#endif

void LeafNode::SetModelViewTransformation(Transformation const & model_view_transformation)
{
	VerifyObject(* this);
	
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
			int shader_index_diff = lhs._program_index - rhs._program_index;
			if (shader_index_diff == 0)
			{
				return lhs._render_depth > rhs._render_depth;
			}
			
			return shader_index_diff < 0;
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

void LeafNode::SetProgramIndex(ProgramIndex::type program_index)
{
	_program_index = program_index;
}

ProgramIndex::type LeafNode::GetProgramIndex() const
{
	return _program_index;
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

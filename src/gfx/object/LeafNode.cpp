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
, _render_sort_key(0)
, _layer(layer)
, _is_opaque(true)
{
}

void LeafNode::SetModelViewTransformation(Transformation const & model_view_transformation)
{
	_model_view_transformation = model_view_transformation;
	
	Transformation::Matrix const & matrix = _model_view_transformation.GetMatrix();
	Scalar depth = matrix[1][3];
	
	_render_sort_key = static_cast<float>(_is_opaque ? - depth : depth);
}

LeafNode::Transformation const & LeafNode::GetModelViewTransformation() const
{
	return _model_view_transformation;
}

bool gfx::operator < (LeafNode const & lhs, LeafNode const & rhs)
{
	return lhs._render_sort_key < rhs._render_sort_key;
}

Layer::type LeafNode::GetLayer() const
{
	return _layer;
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

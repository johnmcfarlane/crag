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
: Object(leaf, ToMap(layer))
, _render_sort_key(0)
, _is_opaque(true)
{
}

LeafNode::LeafNode(Layer::Map::type layers)
: Object(leaf, layers)
, _render_sort_key(0)
, _is_opaque(true)
{
}

void LeafNode::SetModelViewTransformation(Transformation const & model_view_transformation)
{
	_model_view_transformation = model_view_transformation;
	
	Transformation::Matrix const & matrix = _model_view_transformation.GetMatrix();
	Scalar depth = matrix[1][3];
	
	_render_sort_key = _is_opaque ? - depth : depth;
}

gfx::Transformation const & LeafNode::GetModelViewTransformation() const
{
	return _model_view_transformation;
}

bool gfx::operator < (LeafNode const & lhs, LeafNode const & rhs)
{
	return lhs._render_sort_key < rhs._render_sort_key;
}

bool LeafNode::IsOpaque() const
{
	return _is_opaque;
}

void LeafNode::SetIsOpaque(bool is_opaque)
{
	_is_opaque = is_opaque;
}

bool LeafNode::GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, RenderRange & range) const 
{ 
	return false;
}

LeafNode::PreRenderResult LeafNode::PreRender()
{
	return ok;
}

void LeafNode::Render() const
{
	Assert(false);
}

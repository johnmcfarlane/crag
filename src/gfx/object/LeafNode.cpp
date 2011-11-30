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

LeafNode::LeafNode(Layer::type layer)
: Object(leaf, ToMap(layer))
{
}

LeafNode::LeafNode(Layer::Map::type layers)
: Object(leaf, layers)
{
}

bool LeafNode::GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const 
{ 
	return false;
}

void LeafNode::PreRender()
{
	Assert(false);
}

void LeafNode::Render(Layer::type layer, Scene const & scene) const
{
	Assert(false);
}

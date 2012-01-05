//
//  gfx/object/Planet.cpp
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Planet.h"

#include "geom/Ray.h"
#include "geom/Transformation.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

Planet::Planet()
: LeafNode(Layer::foreground)
{
}

void Planet::Update(UpdateParams const & params, Renderer & renderer)
{
	_salient = params;
}

bool Planet::GetRenderRange(RenderRange & range) const 
{
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix const & transformation_matrix = transformation.GetMatrix();
	
	Scalar depth = transformation_matrix[1][3];
	
	range.y = depth + _salient._radius_max;
	range.x = depth - _salient._radius_max;
	
	return true;
}

void Planet::Render(Renderer const & renderer) const 
{ 
	// actual drawing is taken care of by the formation manager
}

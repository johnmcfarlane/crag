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

#include "gfx/Renderer.h"
#include "gfx/SphereQuad.h"

#include "geom/Ray.h"
#include "geom/Transformation.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

Planet::Planet(Scalar sea_level)
: LeafNode(Layer::foreground, ProgramIndex::sphere)
, _sea_level(sea_level)
{
}

void Planet::Update(UpdateParams const & params, Renderer & renderer)
{
	_salient = params;
}

gfx::Transformation const & Planet::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override
{
	SphereQuad const & sphere_quad = renderer.GetSphereQuad();
	return sphere_quad.Transform(model_view, scratch);
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
	gl::SetColor(Color4f(.5f,.5f,.9f,.5f).GetArray());
	
	Transformation const & transformation = GetModelViewTransformation();
	SphereQuad const & sphere_quad = renderer.GetSphereQuad();
	sphere_quad.Draw(transformation);
}

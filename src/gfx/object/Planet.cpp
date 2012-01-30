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
#include "gfx/ResourceManager.h"
#include "gfx/Quad.h"

#include "geom/Ray.h"
#include "geom/Transformation.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

Planet::Planet(Scalar sea_level)
: LeafNode(Layer::foreground)
, _sea_level(sea_level)
{
}

bool Planet::Init(Renderer & renderer)
{
	ResourceManager & resource_manager = renderer.GetResourceManager();
	Program const * sphere_program = resource_manager.GetProgram(ProgramIndex::fog);
	SetProgram(sphere_program);
	
	return true;
}

void Planet::Update(UpdateParams const & params, Renderer & renderer)
{
	_salient = params;
}

gfx::Transformation const & Planet::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetMeshResource());
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
	if (_sea_level < _salient._radius_min)
	{
		return;
	}
	
//	// TODO:
//	Vector3f color(.5f,.5f,.9f);
//	
//	FogProgram const & fog_program = static_cast<FogProgram const &>(renderer.GetProgram());
//
//	Transformation const & transformation = GetModelViewTransformation();
//	Quad const & sphere_quad = renderer.GetQuad();
//	sphere_quad.DrawFog(transformation, color, 0.01);
}

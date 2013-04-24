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

#include "gfx/Program.h"
#include "gfx/Quad.h"
#include "gfx/Engine.h"
#include "gfx/ResourceManager.h"

#include "geom/Ray.h"
#include "geom/Transformation.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Planet member definitions

DEFINE_POOL_ALLOCATOR(Planet, 3);

Planet::Planet(LeafNode::Init const & init, Transformation const & local_transformation, Scalar radius)
	: LeafNode(init, local_transformation, Layer::foreground)
, _sea_level(radius)
{
	ResourceManager & resource_manager = init.engine.GetResourceManager();
	Program * sphere_program = resource_manager.GetProgram(ProgramIndex::fog);
	SetProgram(sphere_program);
}

void Planet::SetRadiusMinMax(Scalar radius_min, Scalar radius_max)
{
	_radius_min = radius_min;
	_radius_max = radius_max;
}

void Planet::UpdateModelViewTransformation(Transformation const & model_view)
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetMeshResource());
	SetModelViewTransformation(sphere_quad.CalculateModelViewTransformation(model_view));
}

bool Planet::GetRenderRange(RenderRange & range) const 
{
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix44 const & transformation_matrix = transformation.GetMatrix();
	
	Scalar depth = transformation_matrix[1][3];
	
	range.y = depth + _radius_max;
	range.x = depth - _radius_max;
	
	return true;
}

void Planet::Render(Engine const &) const
{
	if (_sea_level < _radius_min)
	{
		return;
	}
	
//	FogProgram const & fog_program = static_cast<FogProgram const &>(ref(GetProgram()));
//	Transformation const & transformation = GetModelViewTransformation();
//	Color4f color(.5f,.5f,.9f);
//	fog_program.SetUniforms(transformation, color, .5f);
//
//	Quad const & sphere_quad = static_cast<Quad const &>(ref(GetMeshResource()));
//	sphere_quad.Draw();
}

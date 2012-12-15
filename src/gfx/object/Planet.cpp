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

Planet::Planet(LeafNode::Init const & init, Scalar sea_level)
: LeafNode(init, Layer::foreground)
, _sea_level(sea_level)
{
	ResourceManager & resource_manager = init.engine.GetResourceManager();
	Program const * sphere_program = resource_manager.GetProgram(ProgramIndex::fog);
	SetProgram(sphere_program);
}

void Planet::Update(UpdateParams const & params)
{
	_salient = params;
}

gfx::Transformation const & Planet::Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetMeshResource());
	return sphere_quad.Transform(model_view, scratch);
}

bool Planet::GetRenderRange(RenderRange & range) const 
{
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix44 const & transformation_matrix = transformation.GetMatrix();
	
	Scalar depth = transformation_matrix[1][3];
	
	range.y = depth + _salient._radius_max;
	range.x = depth - _salient._radius_max;
	
	return true;
}

void Planet::Render(gfx::Engine const & renderer) const
{
	if (_sea_level < _salient._radius_min)
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

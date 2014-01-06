//
//  gfx/object/Ball.cpp
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Ball.h"

#include "gfx/axes.h"
#include "gfx/Program.h"
#include "gfx/Engine.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"
#include "gfx/Quad.h"

#include "geom/Transformation.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


using namespace gfx;

DEFINE_POOL_ALLOCATOR(Ball, 100);

Ball::Ball(LeafNode::Init const & init, Transformation const & local_transformation, float radius, Color4f const & color)
: LeafNode(init, local_transformation, Layer::foreground)
, _color(color)
, _radius(radius)
{
	CRAG_VERIFY_EQUAL(_color.a, 1);
	
	ResourceManager & resource_manager = init.engine.GetResourceManager();

	Program * sphere_program = resource_manager.GetProgram(ProgramIndex::sphere);
	SetProgram(sphere_program);
	
	auto & sphere_quad = resource_manager.GetVbo(VboIndex::sphere_quad);
	SetVboResource(& sphere_quad);
}

void Ball::UpdateModelViewTransformation(Transformation const & model_view)
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetVboResource());
	Transformation model_view_transformation = sphere_quad.CalculateModelViewTransformation(model_view, _radius);

	SetModelViewTransformation(model_view_transformation);
}

bool Ball::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Scalar depth = GetDepth(transformation);
	
	range[0] = depth - _radius;
	range[1] = depth + _radius;
	
	return true;
}

void Ball::Render(Engine const & renderer) const
{
	// Pass rendering details to the shader program.
	auto program = renderer.GetCurrentProgram();
	if (program)
	{
		DiskProgram const & sphere_program = static_cast<DiskProgram const &>(* program);
		Transformation const & transformation = GetModelViewTransformation();
		sphere_program.SetUniforms(transformation, _radius, _color);
	}

	// Draw the quad.
	Quad const & sphere_quad = static_cast<Quad const &>(* GetVboResource());
	sphere_quad.Draw();
}

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

#include "gfx/Program.h"
#include "gfx/Renderer.h"
#include "gfx/Scene.h"
#include "gfx/Quad.h"

#include "geom/Transformation.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


using namespace gfx;


Ball::Ball(Color4f const & color)
: LeafNode(Layer::foreground, ProgramIndex::sphere)
, _color(color)
{
}

gfx::Transformation const & Ball::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override
{
	Quad const & sphere_quad = renderer.GetSphereQuad();
	return sphere_quad.Transform(model_view, scratch);
}

bool Ball::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix const & transformation_matrix = transformation.GetMatrix();
	Scalar depth = transformation_matrix[1][3];

	Scalar radius = transformation.GetScale().x;
	
	range[0] = depth - radius;
	range[1] = depth + radius;
	
	return true;
}

void Ball::Render(Renderer const & renderer) const
{
	// Pass rendering details to the shader program.
	SphereProgram const & sphere_program = static_cast<SphereProgram const &>(ref(renderer.GetCurrentProgram()));
	Transformation const & transformation = GetModelViewTransformation();
	sphere_program.SetUniforms(transformation, _color);
	
	// Draw the quad.
	Quad const & sphere_quad = renderer.GetSphereQuad();
	sphere_quad.Draw();
}

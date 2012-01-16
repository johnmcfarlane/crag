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
#include "gfx/SphereMesh.h"
#include "gfx/SphereQuad.h"

#include "glpp/glpp.h"

#include "geom/Transformation.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


CONFIG_DECLARE (enable_sphere_shader, bool);


namespace
{
	int Choose()
	{
		return enable_sphere_shader ? 1 : 0;
	}
}


using namespace gfx;


Ball::Ball(Color4f const & color)
: LeafNode(Layer::foreground, ProgramIndex::sphere)
, _color(color)
{
}

gfx::Transformation const & Ball::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override
{
	switch (Choose())
	{
		default:
			Assert(false);
		case 0:
			return model_view;
			
		case 1:
		{
			SphereQuad const & sphere_quad = renderer.GetSphereQuad();
			return sphere_quad.Transform(model_view, scratch);
		}
	}
}

bool Ball::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix const & transformation_matrix = transformation.GetMatrix();
	Scalar depth = transformation_matrix[1][3];

	Scalar radius = SphereMesh::CalculateRadius(transformation);
	
	range[0] = depth - radius;
	range[1] = depth + radius;
	
	return true;
}

// Perform any necessary preparation for rendering.
LeafNode::PreRenderResult Ball::PreRender(Renderer const & renderer)
{
	switch (Choose())
	{
		default:
			Assert(false);
		case 0:
		{
			SetProgramIndex(ProgramIndex::poly);
			break;
		}
			
		case 1:
		{
			SetProgramIndex(ProgramIndex::sphere);
			break;
		}
	}
	
	return ok;
}

void Ball::Render(Renderer const & renderer) const
{
	Transformation const & transformation = GetModelViewTransformation();
	switch (Choose())
	{
		default:
			Assert(false);
		case 0:
		{
			gl::SetColor(_color.r, _color.g, _color.b);
			SphereMesh const & sphere_mesh = renderer.GetSphereMesh();
			sphere_mesh.Draw(transformation);
			break;
		}
		
		case 1:
		{
			SphereProgram const & sphere_program = static_cast<SphereProgram const &>(ref(renderer.GetProgram()));
			sphere_program.SetUniforms(transformation, _color);
			
			SphereQuad const & sphere_quad = renderer.GetSphereQuad();
			sphere_quad.Draw();
			
			break;
		}
	}
}

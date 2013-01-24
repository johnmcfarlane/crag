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
#include "gfx/Engine.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"
#include "gfx/Quad.h"

#include "geom/Transformation.h"

#include "core/app.h"
#include "core/ConfigEntry.h"


using namespace gfx;

DEFINE_POOL_ALLOCATOR(Ball, 100);

Ball::Ball(LeafNode::Init const & init, Color4f const & color)
: LeafNode(init, Layer::foreground)
, _color(Color4f::Black())
{
	_color = color;
	
	ResourceManager const & resource_manager = init.engine.GetResourceManager();

	Program const * sphere_program = resource_manager.GetProgram(ProgramIndex::sphere);
	SetProgram(sphere_program);
	
	MeshResource const & sphere_quad = resource_manager.GetSphereQuad();
	SetMeshResource(& sphere_quad);
}

gfx::Transformation const & Ball::Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	Quad const & sphere_quad = static_cast<Quad const &>(* GetMeshResource());
	return sphere_quad.Transform(model_view, scratch);
}

bool Ball::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix44 const & transformation_matrix = transformation.GetMatrix();
	Scalar depth = transformation_matrix[1][3];

	Scalar radius = transformation.GetScale().x;
	
	range[0] = depth - radius;
	range[1] = depth + radius;
	
	return true;
}

void Ball::Render(gfx::Engine const & renderer) const
{
	// Pass rendering details to the shader program.
	Program const & program = ref(renderer.GetCurrentProgram());
	SphereProgram const & sphere_program = static_cast<SphereProgram const &>(program);
	Transformation const & transformation = GetModelViewTransformation();
	sphere_program.SetUniforms(transformation, _color);
	
	// Draw the quad.
	Quad const & sphere_quad = static_cast<Quad const &>(* GetMeshResource());
	sphere_quad.Draw();
}

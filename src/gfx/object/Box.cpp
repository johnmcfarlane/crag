//
//  Box.cpp
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Box.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/Program.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"
#include "gfx/VboResource.h"

using namespace gfx;

DEFINE_POOL_ALLOCATOR(Box, 100);

Box::Box(LeafNode::Init const & init, Transformation const & local_transformation, Vector3 const & dimensions, Color4f const & color)
: LeafNode(init, local_transformation, Layer::foreground)
, _color(color)
, _dimensions(dimensions)
{
	ASSERT(_color.a == 1);
	
	ResourceManager & resource_manager = init.engine.GetResourceManager();
	
	Program * poly_program = resource_manager.GetProgram(ProgramIndex::poly);
	SetProgram(poly_program);
	
	VboResource const & cuboid_mesh = resource_manager.GetVbo(VboIndex::cuboid_mesh);
	SetVboResource(& cuboid_mesh);
}

bool Box::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Scalar depth = GetDepth(transformation);

	// This could be improved by sampling each of the 8 corners of the box
	// but it probably isn't worth the clock cycles to do that.
	float radius;
	{
		radius = float(Length(_dimensions) * .5);
	}

	range[0] = depth - radius;
	range[1] = depth + radius;
	
	return true;
}

void Box::Render(Engine const & renderer) const
{
	GL_VERIFY;
	
	// Pass rendering details to the shader program.
	auto program = renderer.GetCurrentProgram();
	if (program)
	{
		PolyProgram const & poly_program = static_cast<PolyProgram const &>(* program);

		bool fragment_lighting = renderer.GetFragmentLighting();
		bool flat_shaded = renderer.GetFlatShaded();
		poly_program.SetUniforms(_color, fragment_lighting, flat_shaded);
	}

	auto & cuboid = * GetVboResource();
	cuboid.Draw();
	
	GL_VERIFY;
}

void Box::UpdateModelViewTransformation(Transformation const & model_view)
{
	Transformation scaled(model_view.GetTranslation(), model_view.GetRotation(), _dimensions);
	SetModelViewTransformation(scaled);
}

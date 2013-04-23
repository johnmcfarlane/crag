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

#include "gfx/Cuboid.h"
#include "gfx/Engine.h"
#include "gfx/Program.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"

#include "core/Random.h"


using namespace gfx;

DEFINE_POOL_ALLOCATOR(Box, 100);

Box::Box(LeafNode::Init const & init, Transformation const & local_transformation, Color4f const & color)
: LeafNode(init, local_transformation, Layer::foreground)
, _color(color)
{
	SetIsOpaque(_color.a == 255);
	
	ResourceManager & resource_manager = init.engine.GetResourceManager();
	
	Program * poly_program = resource_manager.GetProgram(ProgramIndex::poly);
	SetProgram(poly_program);
	
	MeshResource const & cuboid_mesh = resource_manager.GetCuboid();
	SetMeshResource(& cuboid_mesh);
}

bool Box::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix44 const & transformation_matrix = transformation.GetMatrix();
	Scalar depth = transformation_matrix[1][3];

	// This could be improved by sampling each of the 8 corners of the box
	// but it probably isn't worth the clock cycles to do that.
	float radius;
	{
		auto size = transformation.GetScale();
		radius = float(Length(size) * .5);
	}

	range[0] = depth - radius;
	range[1] = depth + radius;
	
	return true;
}

void Box::Render(Engine const &) const
{
	GL_VERIFY;
	
	// Pass rendering details to the shader program.
	Program const & program = ref(renderer.GetCurrentProgram());
	PolyProgram const & poly_program = static_cast<PolyProgram const &>(program);

	bool fragment_lighting = renderer.GetFragmentLighting();
	bool flat_shaded = renderer.GetFlatShaded();
	poly_program.SetUniforms(_color, fragment_lighting, flat_shaded);
	
	Cuboid const & cuboid = static_cast<Cuboid const &>(* GetMeshResource());
	cuboid.Draw();
	
	GL_VERIFY;
}

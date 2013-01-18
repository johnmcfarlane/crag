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
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"

#include "core/Random.h"


using namespace gfx;

DEFINE_POOL_ALLOCATOR(Box, 10);

gfx::Box::Box(LeafNode::Init const & init, Color4f const & color)
: LeafNode(init, Layer::foreground)
, _color(color)
{
	SetIsOpaque(_color.a == 255);
	
	ResourceManager & resource_manager = init.engine.GetResourceManager();
	
	Program const * poly_program = resource_manager.GetProgram(ProgramIndex::poly);
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

void Box::Render(gfx::Engine const & renderer) const
{
	GL_VERIFY;
	
	glColor4fv(_color.GetArray());
	
	Cuboid const & cuboid = static_cast<Cuboid const &>(* GetMeshResource());
	cuboid.Draw();
	
	GL_VERIFY;
}

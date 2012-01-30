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
#include "gfx/Renderer.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"

#include "core/Random.h"


using namespace gfx;


Box::Box(Color4f color)
: LeafNode(Layer::foreground)
, _color(color)
{
	SetIsOpaque(_color.a == 255);
}

bool Box::Init(Renderer & renderer)
{
	ResourceManager & resource_manager = renderer.GetResourceManager();
	
	Program const * poly_program = resource_manager.GetProgram(ProgramIndex::poly);
	SetProgram(poly_program);
	
	MeshResource const & cuboid_mesh = resource_manager.GetCuboid();
	SetMeshResource(& cuboid_mesh);
	
	return true;
}

bool Box::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix const & transformation_matrix = transformation.GetMatrix();
	Scalar depth = transformation_matrix[1][3];

	// This could be improved by sampling each of the 8 corners of the box
	// but it probably isn't worth the clock cycles to do that.
	float radius;
	{
		Vector3 size = transformation.GetScale();
		radius = float(Length(size) * .5);
	}

	range[0] = depth - radius;
	range[1] = depth + radius;
	
	return true;
}

void Box::Render(Renderer const & renderer) const
{
	GL_VERIFY;
	
	glColor4fv(_color.GetArray());
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	Cuboid const & cuboid = static_cast<Cuboid const &>(* GetMeshResource());
	cuboid.Draw();
	
	GL_VERIFY;
}

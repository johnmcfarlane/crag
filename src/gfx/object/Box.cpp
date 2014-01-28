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
#include "gfx/LitVertex.h"
#include "gfx/Mesh.h"
#include "gfx/Program.h"
#include "gfx/Scene.h"
#include "gfx/ShadowVolume.h"
#include "gfx/VboResource.h"

#include "core/ResourceManager.h"

using namespace gfx;

DEFINE_POOL_ALLOCATOR(Box, 100);

Box::Box(LeafNode::Init const & init, Transformation const & local_transformation, Vector3 const & dimensions, Color4f const & color)
: LeafNode(init, local_transformation, Layer::foreground, true, true)
, _color(color)
, _dimensions(dimensions)
{
	ASSERT(_color.a == 1);
	
	auto const & resource_manager = crag::core::ResourceManager::Get();
	
	auto const & poly_program = * resource_manager.GetHandle<PolyProgram>("PolyProgram");
	SetProgram(& poly_program);
	
	auto const & cuboid_mesh = * resource_manager.GetHandle<LitVboResource>("CuboidVbo");
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

		auto fragment_lighting = renderer.GetFragmentLightingEnabled();
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

void Box::GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const
{
	auto & resource_manager = crag::core::ResourceManager::Get();
	auto const & cuboid_mesh = * resource_manager.GetHandle<PlainMesh>("CuboidPlainMesh");
	
	auto light_position = light.GetModelTransformation().GetTranslation();
	auto box_transformation = GetModelTransformation();
	auto box_position = box_transformation.GetTranslation();
	auto box_to_light = light_position - box_position;
	auto rotated_box_to_light = geom::Inverse(box_transformation.GetRotation()) * box_to_light;
	
	auto shadow_volume_mesh = GenerateShadowVolumeMesh(cuboid_mesh, rotated_box_to_light);

	shadow_volume.Set(shadow_volume_mesh);
}

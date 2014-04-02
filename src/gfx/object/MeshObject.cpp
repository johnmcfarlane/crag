//
//  gfx/object/MeshObject.cpp
//  crag
//
//  Created by John McFarlane on 2014-01-27.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MeshObject.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/GenerateShadowVolumeMesh.h"
#include "gfx/IndexedVboResource.h"
#include "gfx/Program.h"

#include "core/ResourceManager.h"

using namespace gfx;

DEFINE_POOL_ALLOCATOR(MeshObject, 100);

////////////////////////////////////////////////////////////////////////////////
// gfx::MeshObject member definitions

MeshObject::MeshObject(Init const & init, Transformation const & local_transformation, Color4f const & color, Vector3 const & scale, LitVboHandle const & lit_vbo, PlainMeshHandle const & plain_mesh)
: LeafNode(init, local_transformation, Layer::foreground, true, true)
, _color(color)
, _scale(scale)
, _bounding_radius(GetBoundingRadius(* plain_mesh, scale))
, _plain_mesh(plain_mesh)
, _lit_vbo(lit_vbo)
{
	SetVboResource(lit_vbo.get());

	auto const & resource_manager = crag::core::ResourceManager::Get();
	auto const & poly_program = * resource_manager.GetHandle<PolyProgram>("PolyProgram");
	SetProgram(& poly_program);
}

MeshObject::~MeshObject()
{
	// TODO: hack to prevent VBO outliving GL context;
	// but what if there are multiple users of this resource?
	_lit_vbo.Flush();
}

bool MeshObject::GetRenderRange(RenderRange & range) const
{
	Transformation const & transformation = GetModelViewTransformation();
	Scalar depth = GetDepth(transformation);

	range[0] = depth - _bounding_radius;
	range[1] = depth + _bounding_radius;
	
	return true;
}

void MeshObject::Render(Engine const & renderer) const
{
	GL_VERIFY;
	
	// Pass rendering details to the shader program.
	auto program = renderer.GetCurrentProgram();
	if (program)
	{
		PolyProgram const & poly_program = static_cast<PolyProgram const &>(* program);

		auto fragment_lighting = renderer.GetFragmentLightingEnabled();
		poly_program.SetUniforms(_color, fragment_lighting);
	}

	auto & vbo = * GetVboResource();
	vbo.Draw();
	
	GL_VERIFY;
}

void MeshObject::UpdateModelViewTransformation(Transformation const & model_view)
{
	// TODO: _scale and _local_transformation constitutes way too much transformation data
	Transformation transformation(model_view.GetTranslation(), model_view.GetRotation(), _scale);
	SetModelViewTransformation(transformation);
}

void MeshObject::GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const
{
	// TODO: include _scale
	auto light_position = light.GetModelTransformation().GetTranslation();
	auto transformation = GetModelTransformation();
	auto position = transformation.GetTranslation();
	auto to_light = light_position - position;
	auto rotated_to_light = geom::Inverse(transformation.GetRotation()) * to_light;
	
	auto shadow_volume_mesh = GenerateShadowVolumeMesh(* _plain_mesh, rotated_to_light);

	shadow_volume.Set(shadow_volume_mesh);
}

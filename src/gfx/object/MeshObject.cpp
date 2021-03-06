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

#include "core/GlobalResourceManager.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::MeshObject member definitions

MeshObject::MeshObject(Engine & engine, Transformation const & local_transformation, Color4f const & color, Vector3 const & scale, ResourceKey lit_vbo_key, ResourceKey plain_mesh_key)
: Object(engine, local_transformation, Layer::opaque, true)
, _color(color)
, _scale(scale)
, _lit_vbo_key(lit_vbo_key)
{
	_plain_mesh = crag::GlobalResourceManager::GetHandle<PlainMesh>(plain_mesh_key);
	_bounding_radius = GetBoundingRadius(* _plain_mesh, scale);
	
	auto & gfx_resource_manager = engine.GetResourceManager();
	_lit_vbo = gfx_resource_manager.GetHandle<VboResource>(_lit_vbo_key);
	auto poly_program = gfx_resource_manager.GetHandle<PolyProgram>("PolyProgram");
	SetProgram(poly_program);

	SetVboResource(_lit_vbo);
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
		poly_program.SetUniforms(_color);
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

bool MeshObject::GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const
{
	auto light_position = light.GetModelTransformation().GetTranslation();
	auto transformation = GetModelTransformation();
	auto position = transformation.GetTranslation();
	auto to_light = light_position - position;
	auto rotated_to_light = geom::Inverse(transformation.GetRotation()) * to_light / _scale;
	
	auto shadow_volume_mesh = GenerateShadowVolumeMesh(* _plain_mesh, rotated_to_light);
	if (shadow_volume_mesh.empty())
	{
		// TODO: Fix for saucer mesh
		return true;
	}

	shadow_volume.Set(shadow_volume_mesh);
	return true;
}

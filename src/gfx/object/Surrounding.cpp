//
//  gfx/object/Surrounding.cpp
//  crag
//
//  Created by John McFarlane on 8/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Surrounding.h"

#include "gfx/Engine.h"
#include "gfx/Messages.h"
#include "gfx/Program.h"
#include "gfx/ResourceManager.h"

#include "gfx/object/Light.h"

#include "form/Engine.h"
#include "form/Surrounding.h"

#include "core/ConfigEntry.h"
#include "core/Statistics.h"


using namespace gfx;

namespace
{
	STAT (num_polys, std::size_t, .05f);
	STAT (num_quats_used, std::size_t, 0.15f);
	
	CONFIG_DEFINE (formation_emission, Color4f, Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_ambient, Color4f, Color4f(0.05f));
	CONFIG_DEFINE (formation_diffuse, Color4f, Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_specular, float, 0.0f);
	CONFIG_DEFINE (formation_shininess, float, 0.0f);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Surrounding member definitions

Surrounding::Surrounding(LeafNode::Init const & init, int max_num_quaterne)
: LeafNode(init, Transformation::Matrix44::Identity(), Layer::foreground, true, false)
, _generation(max_num_quaterne)
, _max_num_quaterne(max_num_quaterne)
{
	ResourceManager & resource_manager = init.engine.GetResourceManager();
	Program * poly_program = resource_manager.GetProgram(ProgramIndex::poly);
	SetProgram(poly_program);

	auto & mesh_resource = _generation.GetVboResource();
	ASSERT(! mesh_resource.IsBound());
	SetVboResource(& mesh_resource);
}

Surrounding::~Surrounding()
{
	CRAG_VERIFY(* this);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Surrounding, object)
	CRAG_VERIFY(static_cast<Surrounding::super const &>(object));

	CRAG_VERIFY(object._mesh);
	if (object._mesh)
	{
		CRAG_VERIFY(* object._mesh);
	}
	
	CRAG_VERIFY(object._generation);

	auto & mesh_resource = object._generation.GetVboResource();
	CRAG_VERIFY_OP(! mesh_resource.IsInitialized(), ||, ! mesh_resource.IsBound());
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Surrounding::UpdateModelViewTransformation(Transformation const & model_view)
{
	auto & generation_origin = _generation.GetOrigin();
	auto & gfx_origin = GetEngine().GetOrigin();
	auto offset = generation_origin - gfx_origin;

	SetModelViewTransformation(model_view * Transformation(geom::Cast<float>(offset)));
}

void Surrounding::SetMesh(std::shared_ptr<form::Mesh> const & mesh)
{
	CRAG_VERIFY_TRUE(mesh);
	
	// if there's already a mesh queued up,
	if (_mesh)
	{
		ASSERT(mesh != _mesh);
		
		// send it back
		ReturnMesh(_mesh);
	}
	
	_mesh = mesh;
	_generation.SetMesh(* mesh);

	// broadcast that this is the current number of quaterne being displayed;
	// means that any performance measurements are taken against this load
	auto num_quaterne = mesh->GetProperties()._num_quaterne;
	if (num_quaterne > 0)
	{
		gfx::NumQuaterneSetMessage message = { num_quaterne };
		Daemon::Broadcast(message);
	}
	
	// state number of polygons/quaterna
	STAT_SET (num_polys, mesh->GetLitMesh().GetIndices().size() / 3);
	STAT_SET (num_quats_used, mesh->GetProperties()._num_quaterne);
}

LeafNode::PreRenderResult Surrounding::PreRender()
{
	CRAG_VERIFY(* this);
	
#if ! defined(NDEBUG)
	Debug::AddBasis(geom::Cast<float>(_generation.GetOrigin()), 1.);
#endif

	return ok;
}

void Surrounding::GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const
{
	if (! _mesh)
	{
		return;
	}
	
	if (! shadow_volume.IsInitialized())
	{
		shadow_volume = ShadowVolume(
			_max_num_quaterne * form::Surrounding::num_verts_per_quaterna * 2, 
			_max_num_quaterne * form::Surrounding::num_indices_per_quaterna);
	}

	auto gfx_light_position = light.GetModelTransformation().GetTranslation();
	auto form_light_position = GfxToForm(gfx_light_position);

	auto shadow_volume_mesh = GenerateShadowVolumeMesh(_mesh->GetLitMesh(), form_light_position);
	shadow_volume.Set(shadow_volume_mesh);
}

void Surrounding::Render(Engine const & renderer) const
{
	auto & mesh_resource = _generation.GetVboResource();
	if (mesh_resource.GetNumVertices() == 0)
	{
		return;
	}
	
	// Pass rendering details to the shader program.
	auto program = renderer.GetCurrentProgram();
	if  (program)
	{
		PolyProgram const & poly_program = static_cast<PolyProgram const &>(* program);

		bool fragment_lighting = renderer.GetFragmentLightingEnabled();
		bool flat_shaded = renderer.GetFlatShaded();
		poly_program.SetUniforms(Color4f::White(), fragment_lighting, flat_shaded);
	}
	
	// Draw the mesh!
	mesh_resource.Draw();
}

void Surrounding::ReturnMesh(std::shared_ptr<form::Mesh> const & mesh)
{
	form::Daemon::Call([mesh] (form::Engine & engine) {
		engine.OnSetMesh(mesh);
	});
}

Vector3 Surrounding::GfxToForm(Vector3 const & position) const
{
	auto & form_origin = _generation.GetOrigin();
	auto & gfx_origin = GetEngine().GetOrigin();
	return geom::Convert(position, gfx_origin, form_origin);
}

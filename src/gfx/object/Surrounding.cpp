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

#include "form/Engine.h"

#include "core/ConfigEntry.h"
#include "core/ResourceManager.h"
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
	CONFIG_DEFINE (relief_enabled, bool, false);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Surrounding member definitions

Surrounding::Surrounding(LeafNode::Init const & init)
: LeafNode(init, Transformation::Matrix44::Identity(), Layer::foreground, true, false)
{
	auto const & resource_manager = crag::core::ResourceManager::Get();
	auto const & poly_program = * resource_manager.GetHandle<PolyProgram>("PolyProgram");
	SetProgram(& poly_program);

	ASSERT(! _vbo_resource.IsInitialized() || ! _vbo_resource.IsBound());
	SetVboResource(& _vbo_resource);
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
	
	CRAG_VERIFY_OP(! object._vbo_resource.IsInitialized(), ||, ! object._vbo_resource.IsBound());

	CRAG_VERIFY(object._vbo_resource);
	CRAG_VERIFY(object._properties);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Surrounding::UpdateModelViewTransformation(Transformation const & model_view)
{
	auto & gfx_origin = GetEngine().GetOrigin();
	auto offset = _properties._origin - gfx_origin;

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
	
	// do some assignment
	_mesh = mesh;
	_properties = mesh->GetProperties();
	_vbo_resource.Set(mesh->GetLitMesh());
	
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
	Debug::AddBasis(geom::Cast<float>(_properties._origin), 1.);
#endif

	return ok;
}

void Surrounding::GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const
{
	if (! _mesh)
	{
		return;
	}
	
	auto gfx_light_position = light.GetModelTransformation().GetTranslation();
	auto form_light_position = GfxToForm(gfx_light_position);

	auto shadow_volume_mesh = GenerateShadowVolumeMesh(_mesh->GetLitMesh(), form_light_position);
	shadow_volume.Set(shadow_volume_mesh);
}

void Surrounding::Render(Engine const & renderer) const
{
	if (_vbo_resource.GetNumIndices() == 0)
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
		poly_program.SetUniforms(Color4f::White(), fragment_lighting, flat_shaded, relief_enabled);
	}
	
	// Draw the mesh!
	_vbo_resource.Draw();
}

void Surrounding::ReturnMesh(std::shared_ptr<form::Mesh> const & mesh)
{
	form::Daemon::Call([mesh] (form::Engine & engine) {
		engine.OnSetMesh(mesh);
	});
}

Vector3 Surrounding::GfxToForm(Vector3 const & position) const
{
	auto & form_origin = _properties._origin;
	auto & gfx_origin = GetEngine().GetOrigin();
	return geom::Convert(position, gfx_origin, form_origin);
}

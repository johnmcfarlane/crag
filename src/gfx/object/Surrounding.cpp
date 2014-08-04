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

#include "gfx/Debug.h"
#include "gfx/Engine.h"
#include "gfx/GenerateShadowVolumeMesh.h"
#include "gfx/Messages.h"
#include "gfx/NonIndexedVboResource.h"
#include "gfx/Program.h"

#if defined(CRAG_FORM_FLAT_SHADE)
#include "gfx/NonIndexedVboResource.h"
#else
#include "gfx/IndexedVboResource.h"
#endif

#include "form/Engine.h"
#include "form/Mesh.h"

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
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Surrounding member definitions

Surrounding::Surrounding(Engine & engine)
: Object(engine, Transformation::Matrix44::Identity(), Layer::opaque, false)
{
	auto const & resource_manager = crag::core::ResourceManager::Get();
	auto const poly_program = resource_manager.GetHandle<PolyProgram>("PolyProgram");
	SetProgram(poly_program);

	auto & vbo_resource = * new VboResource;
	vbo_resource.Set(form::Mesh::LitMesh());
	SetVboResource(& vbo_resource);

	CRAG_VERIFY(* this);
}

Surrounding::~Surrounding()
{
	CRAG_VERIFY(* this);

	delete GetVboResource();
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Surrounding, object)
	CRAG_VERIFY(static_cast<Object const &>(object));

	CRAG_VERIFY_TRUE(object.GetVboResource());
	auto const & vbo_resource = core::StaticCast<VboResource const &>(* object.GetVboResource());
	CRAG_VERIFY_TRUE(vbo_resource.IsInitialized());

	CRAG_VERIFY(object._mesh);
	if (object._mesh)
	{
		auto const & mesh = * object._mesh;
		CRAG_VERIFY(mesh);
		CRAG_VERIFY_EQUAL(mesh.GetLitMesh().empty(), vbo_resource.empty());
	}
	else
	{
		CRAG_VERIFY_TRUE(vbo_resource.empty());
	}

	CRAG_VERIFY(object._properties);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Surrounding::UpdateModelViewTransformation(Transformation const & model_view)
{
	auto & gfx_space = GetEngine().GetSpace();
	auto offset = _properties._space - gfx_space;

	SetModelViewTransformation(model_view * Transformation(geom::Cast<float>(offset)));
}

void Surrounding::SetMesh(std::shared_ptr<form::Mesh> const & mesh)
{
	CRAG_VERIFY(* this);
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
	
	auto const & lit_mesh = mesh->GetLitMesh();
	auto const & vbo_resource = core::StaticCast<VboResource const>(* GetVboResource());
	auto & mutable_vbo_resource = const_cast<VboResource &>(vbo_resource);
	mutable_vbo_resource.Set(lit_mesh);
	
	// broadcast that this is the current number of quaterne being displayed;
	// means that any performance measurements are taken against this load
	auto num_quaterne = mesh->GetProperties()._num_quaterne;
	if (num_quaterne > 0)
	{
		gfx::NumQuaterneSetMessage message = { num_quaterne };
		Daemon::Broadcast(message);
	}
	
	// state number of polygons/quaterna
	STAT_SET (num_polys, lit_mesh.size() / 3);
	STAT_SET (num_quats_used, mesh->GetProperties()._num_quaterne);
	CRAG_VERIFY(* this);
}

Object::PreRenderResult Surrounding::PreRender()
{
	CRAG_VERIFY(* this);
	
#if ! defined(NDEBUG)
	Debug::AddBasis(_properties._space.AbsToRel(geom::abs::Vector3::Zero()), 1.);
#endif

	return ok;
}

bool Surrounding::GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const
{
	if (! _mesh)
	{
		return true;
	}
	
	auto gfx_light_position = light.GetModelTransformation().GetTranslation();
	auto form_light_position = GfxToForm(gfx_light_position);

	auto shadow_volume_mesh = GenerateShadowVolumeMesh(_mesh->GetLitMesh(), form_light_position);
	if (shadow_volume_mesh.empty())
	{
		return false;
	}
	
	shadow_volume.Set(shadow_volume_mesh);
	
	return true;
}

void Surrounding::Render(Engine const & renderer) const
{
	CRAG_VERIFY(* this);

	auto const & vbo_resource = core::StaticCast<VboResource const>(* GetVboResource());
	if (vbo_resource.empty())
	{
		return;
	}
	
	// Pass rendering details to the shader program.
	auto program = renderer.GetCurrentProgram();
	if  (program)
	{
		PolyProgram const & poly_program = static_cast<PolyProgram const &>(* program);
		poly_program.SetUniforms(Color4f::White());
	}
	
	// Draw the mesh!
	vbo_resource.Draw();
	CRAG_VERIFY(* this);
}

void Surrounding::ReturnMesh(std::shared_ptr<form::Mesh> const & mesh)
{
	form::Daemon::Call([mesh] (form::Engine & engine) {
		engine.OnSetMesh(mesh);
	});
}

Vector3 Surrounding::GfxToForm(Vector3 const & position) const
{
	auto & form_space = _properties._space;
	auto & gfx_space = GetEngine().GetSpace();
	return geom::Convert(position, gfx_space, form_space);
}

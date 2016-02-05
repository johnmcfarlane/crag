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

#if defined(CRAG_DEBUG)
CONFIG_DECLARE(physics_debug_draw, bool);
#endif

namespace
{
	STAT (num_polys, std::size_t, .05f);
	STAT (num_quats_used, std::size_t, 0.15f);
	
	CONFIG_DEFINE(formation_emission, Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE(formation_ambient, Color4f(0.05f));
	CONFIG_DEFINE(formation_diffuse, Color4f(0.0f, 0.0f, 0.0f));

	char const * vbo_key = "SurroundingMesh";
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Surrounding member definitions

Surrounding::Surrounding(Engine & engine)
: Object(engine, Transformation(), Layer::opaque, false)
{
	auto & resource_manager = engine.GetResourceManager();
	auto const poly_program = resource_manager.GetHandle<PolyProgram>("PolyProgram");
	SetProgram(poly_program);
	
	resource_manager.Register<VboResource>(vbo_key, [&] () {
		return VboResource();
	});
	
	CRAG_VERIFY(* this);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Surrounding, object)
	CRAG_VERIFY(static_cast<Object const &>(object));
	
	auto vbo_resource_handle = object.GetVboResource();

	CRAG_VERIFY(object._mesh);
	if (object._mesh)
	{
		auto const & mesh = * object._mesh;
		CRAG_VERIFY(mesh);
	}
	else
	{
		CRAG_VERIFY_FALSE(vbo_resource_handle);
	}

	CRAG_VERIFY(object._properties);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Surrounding::UpdateModelViewTransformation(Transformation const & model_view)
{
	auto & gfx_space = GetEngine().GetSpace();
	auto offset = _properties._space - gfx_space;

	SetModelViewTransformation(model_view * Transformation(static_cast<Vector3>(offset)));
}

void Surrounding::SetMesh(std::shared_ptr<form::Mesh> const & mesh)
{
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
	
	if (! GetEngine().GetIsSuspended())
	{
		UpdateVbo();
	}
}

Object::PreRenderResult Surrounding::PreRender()
{
	CRAG_VERIFY(* this);
	
#if defined(CRAG_DEBUG)
	Debug::AddBasis(_properties._space.AbsToRel(geom::uni::Vector3::Zero()), 1.);
#endif

	auto vbo_resource_handle = GetVboResource();
	if (vbo_resource_handle)
	{
		auto const & vbo_resource = core::StaticCast<VboResource const>(* vbo_resource_handle);
		if (vbo_resource.empty() && _mesh)
		{
			UpdateVbo();
		}
	}
	
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

#if defined(CRAG_DEBUG)
	if (physics_debug_draw)
	{
		return;
	}
#endif

	auto vbo_resource_handle = GetVboResource();
	if (! vbo_resource_handle)
	{
		return;
	}
	
	auto const & vbo_resource = core::StaticCast<VboResource const>(* vbo_resource_handle);
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

void Surrounding::UpdateVbo()
{
	auto const & mesh = * _mesh;
	auto const & lit_mesh = mesh.GetLitMesh();
	auto const & properties = mesh.GetProperties();

	// lazily create VBO once it arrives from form::Engine
	auto vbo_resource_handle = GetVboResource();
	if (! vbo_resource_handle)
	{
		auto & resource_manager = GetEngine().GetResourceManager();
		vbo_resource_handle = resource_manager.GetHandle<VboResource>(vbo_key);
		ASSERT(vbo_resource_handle);
		
		SetVboResource(vbo_resource_handle);
	}

	auto const & vbo_resource = core::StaticCast<VboResource const>(* vbo_resource_handle);
	auto & mutable_vbo_resource = const_cast<VboResource &>(vbo_resource);
	mutable_vbo_resource.Set(lit_mesh);

	// broadcast that this is the current number of quaterne being displayed;
	// means that any performance measurements are taken against this load
	auto num_quaterne = properties._num_quaterne;
	if (num_quaterne > 0)
	{
		gfx::NumQuaterneSetMessage message = { num_quaterne };
		Daemon::Broadcast(message);
	}
	
	// state number of polygons/quaterna
	STAT_SET (num_polys, lit_mesh.size() / 3);
	STAT_SET (num_quats_used, properties._num_quaterne);
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

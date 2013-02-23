//
//  gfx/object/FormationMesh.cpp
//  crag
//
//  Created by John McFarlane on 8/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "FormationMesh.h"

#include "gfx/Debug.h"
#include "gfx/Engine.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"

#include "form/Engine.h"
#include "form/node/NodeBuffer.h"

#include "scripts/RegulatorScript.h"

#include "applet/Engine.h"

#include "core/ConfigEntry.h"
#include "core/Statistics.h"


using namespace gfx;


namespace
{
	STAT (num_polys, int, .05f);
	STAT (num_quats_used, int, 0.15f);
	
	CONFIG_DEFINE (formation_emission, Color4f, Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_ambient, Color4f, Color4f(0.05f));
	CONFIG_DEFINE (formation_diffuse, Color4f, Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_specular, float, 0.0f);
	CONFIG_DEFINE (formation_shininess, float, 0.0f);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::FormationMesh member definitions

FormationMesh::FormationMesh(LeafNode::Init const & init, size_t max_num_quaterne, smp::Handle<form::RegulatorScript> const & regulator_handle)
: LeafNode(init, Transformation::Matrix44::Identity(), Layer::foreground)
, _queued_mesh(nullptr)
, _pending_mesh(nullptr)
{
	size_t max_num_verts = max_num_quaterne * form::NodeBuffer::num_verts_per_quaterna;
	size_t max_num_indices = max_num_quaterne * form::NodeBuffer::num_indices_per_quaterna;
	
	for (int index = 0; index < 2; ++ index)
	{
		// initialize mesh buffer
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Init();
		mbo.Bind();
		mbo.Resize(max_num_verts, max_num_indices);
		mbo.Unbind();
	}
	
	OnMeshResourceChange();

	ResourceManager & resource_manager = init.engine.GetResourceManager();
	Program const * poly_program = resource_manager.GetProgram(ProgramIndex::poly);
	SetProgram(poly_program);
	
	_regulator_handle = regulator_handle;
}

FormationMesh::~FormationMesh()
{
	for (int index = 0; index < 2; ++ index)
	{
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Deinit();
	}
	
	delete _queued_mesh;
	delete _pending_mesh;
}

#if defined(VERIFY)
void FormationMesh::Verify() const
{
	super::Verify();
	ASSERT(! mbo_buffers.back().IsBound());
}
#endif

void FormationMesh::SetMesh(form::Mesh * const & mesh)
{
	ASSERT(mesh != _queued_mesh);
	ASSERT(mesh != _pending_mesh);
	
	// If there's already a mesh queued up,
	if (_queued_mesh != nullptr)
	{
		// send it back because it's not the newest anymore.
		ReturnMesh(* _queued_mesh);
	}
	
	_queued_mesh = mesh;
}

LeafNode::PreRenderResult FormationMesh::PreRender()
{
	VerifyObject(* this);
	
	FinishBufferUpload();
	BeginBufferUpload();

#if ! defined(NDEBUG)
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	Debug::AddBasis(front_buffer.GetOrigin(), 1.);
#endif
	
	return ok;
}

void FormationMesh::UpdateModelViewTransformation(Transformation const & model_view)
{
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	auto& front_buffer_origin = front_buffer.GetOrigin();
	auto& gfx_origin = GetEngine().GetOrigin();
	auto offset = front_buffer_origin - gfx_origin;

	SetModelViewTransformation(model_view * Transformation(geom::Cast<float>(offset)));
}

void FormationMesh::Render(Engine const & renderer) const
{
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	if (front_buffer.GetNumPolys() == 0)
	{
		return;
	}
	
	// State
	ASSERT(IsEnabled(GL_DEPTH_TEST));
	ASSERT(IsEnabled(GL_COLOR_MATERIAL));
	
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, formation_ambient));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, formation_diffuse));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, Color4f(formation_specular, formation_specular, formation_specular)));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, formation_emission));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, & formation_shininess));
	GL_CALL(glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE));
	
	ASSERT(! IsEnabled(GL_TEXTURE_2D));
	
	// Draw the mesh!
	front_buffer.Draw();
}

bool FormationMesh::BeginBufferUpload()
{
	VerifyObject(* this);

	if (_pending_mesh != nullptr)
	{
		// an upload is already in progress
		return false;
	}
	
	if (_queued_mesh == nullptr)
	{
		// a new mesh is not available to upload
		return false;
	}
	
	form::MeshBufferObject & back_buffer = mbo_buffers.back();
	VerifyObject(* this);
	
	back_buffer.Bind();
	back_buffer.Set(* _queued_mesh);
	back_buffer.Unbind();
	
	_pending_mesh = _queued_mesh;
	_queued_mesh = nullptr;

	return true;
}

void FormationMesh::OnMeshResourceChange()
{
	form::MeshBufferObject & front_buffer = mbo_buffers.front();
	ASSERT(! front_buffer.IsBound());
	SetMeshResource(& front_buffer);
}

bool FormationMesh::FinishBufferUpload()
{
	if (_pending_mesh == nullptr)
	{
		// not currently uploading a mesh
		return false;
	}
	
	// set the newly uploaded mbo as the front one
	mbo_buffers.flip();

	OnMeshResourceChange();
	
	// inform the regulator that following frame information 
	// will relate to a mesh of this number of quaterna.
	int num_quaterne = _pending_mesh->GetProperties()._num_quaterne;
	if (_regulator_handle && num_quaterne > 0)
	{
		_regulator_handle.Call([num_quaterne] (form::RegulatorScript & script) {
			script.SetNumQuaterne(num_quaterne);
		});
	}
	
	// state number of polygons/quaterna
	STAT_SET (num_polys, _pending_mesh->GetNumPolys());
	STAT_SET (num_quats_used, num_quaterne);
	
	// release the pending mesh back to the formation manager
	ReturnMesh(* _pending_mesh);
	_pending_mesh = nullptr;
	
	return true;
}

void FormationMesh::ReturnMesh(form::Mesh & mesh)
{
	form::Daemon::Call([&mesh] (form::Engine & engine) {
		engine.OnSetMesh(mesh);
	});
}

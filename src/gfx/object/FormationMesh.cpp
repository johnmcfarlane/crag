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
#include "gfx/Renderer.h"
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"

#include "form/FormationManager.h"
#include "form/node/NodeBuffer.h"

#include "core/ConfigEntry.h"
#include "core/Statistics.h"


using namespace gfx;


namespace
{
	STAT (num_polys, int, .05f);
	STAT (num_quats_used, int, 0.15f);
	
	CONFIG_DEFINE (formation_emission, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_ambient, gfx::Color4f, gfx::Color4f(0.05f));
	CONFIG_DEFINE (formation_diffuse, gfx::Color4f, gfx::Color4f(0.0f, 0.0f, 0.0f));
	CONFIG_DEFINE (formation_specular, float, 0.0f);
	CONFIG_DEFINE (formation_shininess, float, 0.0f);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::FormationMesh member definitions

FormationMesh::FormationMesh()
: LeafNode(Layer::foreground)
, _queued_mesh(nullptr)
, _pending_mesh(nullptr)
{
}

#if defined(VERIFY)
void FormationMesh::Verify() const
{
	super::Verify();
	Assert(! mbo_buffers.back().IsBound());
}
#endif

bool FormationMesh::Init(Renderer & renderer, InitData const & init_data)
{
	for (int index = 0; index < 2; ++ index)
	{
		// initialize mesh buffer
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Init();
		mbo.Bind();
		mbo.Resize(form::NodeBuffer::max_num_verts, form::NodeBuffer::max_num_indices);
		mbo.Unbind();
	}
	
	OnMeshResourceChange();

	ResourceManager & resource_manager = renderer.GetResourceManager();
	Program const * poly_program = resource_manager.GetProgram(ProgramIndex::poly);
	SetProgram(poly_program);
	
	return true;
}

void FormationMesh::Deinit(Scene & scene)
{
	for (int index = 0; index < 2; ++ index)
	{
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Deinit();
	}
	
#if ! defined(NDEBUG)
	std::cout << "FormationMesh has " << (int)(_queued_mesh != nullptr) + (_pending_mesh != nullptr) << " meshes." << std::endl;
#endif
	
	delete _queued_mesh;
	delete _pending_mesh;
}

void FormationMesh::SetMesh(form::Mesh * const & mesh)
{
	Assert(mesh != _queued_mesh);
	Assert(mesh != _pending_mesh);
	
	// If there's already a mesh queued up,
	if (_queued_mesh != nullptr)
	{
		// send it back because it's not the newest anymore.
		ReturnMesh(* _queued_mesh);
	}
	
	_queued_mesh = mesh;
}

LeafNode::PreRenderResult FormationMesh::PreRender(Renderer const & renderer)
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

gfx::Transformation const & FormationMesh::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	scratch = model_view * sim::Transformation(front_buffer.GetOrigin());
	return scratch;
}

void FormationMesh::Render(Renderer const & renderer) const
{
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	if (front_buffer.GetNumPolys() == 0)
	{
		return;
	}
	
	// State
	Assert(IsEnabled(GL_DEPTH_TEST));
	Assert(IsEnabled(GL_COLOR_MATERIAL));
	
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, formation_ambient));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, formation_diffuse));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx::Color4f(formation_specular, formation_specular, formation_specular)));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, formation_emission));
	GL_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, & formation_shininess));
	GL_CALL(glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE));
	
	Assert(! IsEnabled(GL_TEXTURE_2D));
	
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
	Assert(! front_buffer.IsBound());
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
	form::Daemon::Call<int>(num_quaterne, & form::FormationManager::OnRegulatorSetNumQuaterna);
	
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
	form::Daemon::Call(& mesh, & form::FormationManager::OnSetMesh);
}

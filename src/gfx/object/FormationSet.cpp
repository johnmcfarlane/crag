//
//  gfx/object/FormationSet.cpp
//  crag
//
//  Created by John McFarlane on 8/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "FormationSet.h"

#include "gfx/Debug.h"
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
// gfx::FormationSet member definitions

FormationSet::FormationSet()
: _queued_mesh(nullptr)
, _pending_mesh(nullptr)
{
}

void FormationSet::Init()
{
	for (int index = 0; index < 2; ++ index)
	{
		// initialize mesh buffer
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Init();
		mbo.Bind();
		mbo.Resize(form::NodeBuffer::max_num_verts, form::NodeBuffer::max_num_indices, gl::DYNAMIC_DRAW);
		mbo.Unbind();
	}
}

void FormationSet::Deinit()
{
	for (int index = 0; index < 2; ++ index)
	{
		MboDoubleBuffer::value_type & mbo = mbo_buffers[index];
		mbo.Deinit();
	}
	
#if ! defined(NDEBUG)
	std::cout << "FormationSet has " << (int)(_queued_mesh != nullptr) + (_pending_mesh != nullptr) << " meshes." << std::endl;
#endif
	
	delete _queued_mesh;
	delete _pending_mesh;
}

void FormationSet::Update(UpdateParams const & params)
{
	form::Mesh * mesh = params;
	
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

void FormationSet::PreRender()
{
	FinishBufferUpload();
	BeginBufferUpload();

#if ! defined(NDEBUG)
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	Debug::AddBasis(front_buffer.GetOrigin(), 1.);
#endif
}

void FormationSet::Render(Layer::type layer, Scene const & scene) const
{
	form::MeshBufferObject const & front_buffer = mbo_buffers.front();
	if (front_buffer.GetNumPolys() == 0)
	{
		return;
	}
	
	// When asking the regulator to adjust the number of quaterna.
	//			NodeBuffer & active_buffer = GetActiveScene().GetNodeBuffer();
	//			rendered_num_quaternia = active_buffer.GetNumQuaternaUsed();
	
	// State
	Assert(gl::IsEnabled(GL_DEPTH_TEST));
	Assert(gl::IsEnabled(GL_COLOR_MATERIAL));
	
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, formation_ambient));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, formation_diffuse));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, gfx::Color4f(formation_specular, formation_specular, formation_specular)));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, formation_emission));
	GLPP_CALL(glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, & formation_shininess));
	GLPP_CALL(glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE));
	
	Assert(! gl::IsEnabled(GL_TEXTURE_2D));
	
	// Draw the mesh!
	front_buffer.Bind();
	front_buffer.Activate(scene.GetPov());
	front_buffer.Draw();
	front_buffer.Deactivate();
	GLPP_VERIFY;
}

bool FormationSet::IsInLayer(Layer::type layer) const
{
	return layer == Layer::pre_render
	|| layer == Layer::foreground;
}

bool FormationSet::BeginBufferUpload()
{
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
	
	back_buffer.Bind();
	back_buffer.Set(* _queued_mesh);
	back_buffer.Unbind();
	
	_pending_mesh = _queued_mesh;
	_queued_mesh = nullptr;

	return true;
}

bool FormationSet::FinishBufferUpload()
{
	if (_pending_mesh == nullptr)
	{
		// not currently uploading a mesh
		return false;
	}
	
	// set the newly uploaded mbo as the front one
	mbo_buffers.flip();
	
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

// TODO: Why not called everywhere in class?
void FormationSet::ReturnMesh(form::Mesh & mesh)
{
	form::Daemon::Call(& mesh, & form::FormationManager::OnSetMesh);
}

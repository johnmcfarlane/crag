//
//  gfx/object/FormationMesh.h
//  crag
//
//  Created by John McFarlane on 8/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/LeafNode.h"

#include "form/scene/MeshBufferObject.h"

#include "core/double_buffer.h"


namespace gfx
{
	// the graphical representation of all the formations
	class FormationMesh : public LeafNode
	{
		// types
		typedef LeafNode super;
		typedef core::double_buffer<form::MeshBufferObject> MboDoubleBuffer;
		
	public:
		// TODO: Take Init call out of Renderer/Simulation altogether.
		struct InitData
		{
		};
		
		// functions
		FormationMesh();
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		bool Init(Renderer & renderer, InitData const & init_data);
		void Deinit(Scene & scene) override;

		gfx::Transformation const & Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;

		void SetMesh(form::Mesh * const & mesh);
		
	private:
		PreRenderResult PreRender(Renderer const & renderer) override;
		void Render(Renderer const & renderer) const override;
		
		void OnMeshResourceChange();
		bool FinishBufferUpload();
		bool BeginBufferUpload();
		void ReturnMesh(form::Mesh & mesh);
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// last mesh to arrive that hasn't been looked at yet
		form::Mesh * _queued_mesh;
		
		// mesh that is being uploaded to a buffer object
		form::Mesh * _pending_mesh;
		
		// front buffer is drawable, back buffer is being written iff pending is non-null
		MboDoubleBuffer mbo_buffers;
	};
}


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
		typedef core::double_buffer<form::MeshBufferObject> MboDoubleBuffer;
		
	public:
		typedef form::Mesh * UpdateParams;
		
		// functions
		FormationMesh();
		
		void Init(Scene const & scene) override;
		void Deinit() override;

		Transformation const & Transform(Transformation const & model_view, Transformation & scratch) const override;

		void Update(UpdateParams const & params);
		
	private:
		virtual PreRenderResult PreRender();		
		virtual void Render(Transformation const & transformation, Layer::type layer) const;
		
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


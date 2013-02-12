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


namespace form { DECLARE_CLASS_HANDLE(RegulatorScript); }	// form::RegulatorScriptHandle


namespace gfx
{
	// the graphical representation of all the formations
	class FormationMesh : public LeafNode
	{
		// types
		typedef LeafNode super;
		typedef core::double_buffer<form::MeshBufferObject> MboDoubleBuffer;
		
	public:
		// functions
		FormationMesh(Init const & init, size_t max_num_quaterne, form::RegulatorScriptHandle const & regulator_handle);
		~FormationMesh();
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		virtual Transformation const & Transform(Transformation const & model_view, Transformation & scratch) const override;

		void SetMesh(form::Mesh * const & mesh);
		
	private:
		PreRenderResult PreRender() override;
		void Render(Engine const & renderer) const override;
		
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
		
		// The formation mesh feeds performance-related information to the regulator.
		form::RegulatorScriptHandle _regulator_handle;
	};
}


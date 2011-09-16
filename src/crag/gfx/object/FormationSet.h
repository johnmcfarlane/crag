//
//  gfx/object/FormationSet.h
//  crag
//
//  Created by John McFarlane on 8/31/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/Object.h"

#include "form/scene/MeshBufferObject.h"

#include "glpp/Fence.h"

#include "core/double_buffer.h"


namespace gfx
{
	// the graphical representation of all the formations
	class FormationSet : public Object
	{
		// types
		typedef core::double_buffer<form::MeshBufferObject> MboDoubleBuffer;
		
	public:
		typedef form::Mesh * UpdateParams;
		
		// functions
		FormationSet();
		
		virtual void Init();
		virtual void Deinit();

		void Update(UpdateParams const & params);
		
	private:
		virtual void PreRender();		
		virtual void Render(Layer::type layer, Scene const & scene) const;
		virtual bool IsInLayer(Layer::type layer) const;
		
		bool FinishBufferUpload();
		bool BeginBufferUpload();
		void ReturnMesh(form::Mesh & mesh);
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		
		// last mesh to arrive that hasn't been looked at yet
		form::Mesh * _queued_mesh;
		
		// mesh that is being uploaded to a buffer object
		form::Mesh * _pending_mesh;
		
		// buffer object upload fence
		// TODO: array range extension may improve performance
		gl::Fence _fence;
		
		// front buffer is drawable, back buffer is being written iff pending is non-null
		MboDoubleBuffer mbo_buffers;
	};
}

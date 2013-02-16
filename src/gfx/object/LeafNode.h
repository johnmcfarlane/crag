//
//  LeafNode.h
//  crag
//
//  Created by John McFarlane on 2011-11-23.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Object.h"

#include "gfx/Layer.h"

#include "geom/Transformation.h"


namespace gfx
{
	// forward-declarations
	class Program;
	class LeafNode;
	class MeshResource;

	bool operator < (LeafNode const & lhs, LeafNode const & rhs);

	// An element of the scene which can be drawn.
	class LeafNode : public Object
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		typedef Object super;

		enum PreRenderResult
		{
			ok,
			remove
		};
		
		////////////////////////////////////////////////////////////////////////////////
		// functions

		LeafNode(Init const & init, Transformation const & local_transformation, Layer::type layer);
		
#if defined(VERIFY)
		virtual void Verify() const override;
#endif
		
		virtual LeafNode & CastLeafNodeRef() final;
		virtual LeafNode const & CastLeafNodeRef() const final;
		virtual LeafNode * CastLeafNodePtr() final;
		virtual LeafNode const * CastLeafNodePtr() const final;

		virtual void UpdateModelViewTransformation(Transformation const & model_view_transformation);
		void SetModelViewTransformation(Transformation const & model_view_transformation);
		Transformation const & GetModelViewTransformation() const;

		friend bool operator < (LeafNode const & lhs, LeafNode const & rhs);
		
		Layer::type GetLayer() const;
		
		Program const * GetProgram() const;
		void SetProgram(Program const  * program);
		
		MeshResource const * GetMeshResource() const;
		void SetMeshResource(MeshResource const * mesh_resource);
		
		bool IsOpaque() const;
		void SetIsOpaque(bool is_opaque);
		
		// Return the necessary z-clipping range required to render this object through the given camera.
		virtual bool GetRenderRange(RenderRange & range) const;
		
		// Perform any necessary preparation for rendering.
		virtual PreRenderResult PreRender();
		
		// Draw the object.
		virtual void Render(Engine const & renderer) const;

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		// The resultant transformation.
		Transformation _model_view_transformation;
		
		// This is the list which is sorted in render order.
		DEFINE_INTRUSIVE_LIST(LeafNode, RenderList);
		
		float _render_depth;
		Layer::type _layer;
		Program const * _program;
		MeshResource const * _mesh_resource;
		bool _is_opaque;
	};
}

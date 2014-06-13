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

#include "gfx/ShadowVolume.h"

#include "geom/Transformation.h"


namespace gfx
{
	// forward-declarations
	class LeafNode;
	class Light;
	class Program;
	class VboResource;

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

		LeafNode(Init const & init, Transformation const & local_transformation, Layer layer, bool casts_shadow = false);
		
		CRAG_VERIFY_INVARIANTS_DECLARE(LeafNode);
		
		virtual LeafNode & CastLeafNodeRef() final;
		virtual LeafNode const & CastLeafNodeRef() const final;
		virtual LeafNode * CastLeafNodePtr() final;
		virtual LeafNode const * CastLeafNodePtr() const final;

		virtual void UpdateModelViewTransformation(Transformation const & model_view_transformation);
		void SetModelViewTransformation(Transformation const & model_view_transformation);
		Transformation const & GetModelViewTransformation() const;
		virtual Transformation const & GetShadowModelViewTransformation() const;

		friend bool operator < (LeafNode const & lhs, LeafNode const & rhs);
		
		Layer GetLayer() const;
		
		Program const * GetProgram() const;
		void SetProgram(Program const * program);
		
		VboResource const * GetVboResource() const;
		void SetVboResource(VboResource const * mesh_resource);
		
		bool CastsShadow() const;
		
		// Return the necessary z-clipping range required to render this object through the given camera.
		virtual bool GetRenderRange(RenderRange & range) const;
		
		// Perform any necessary preparation for rendering.
		virtual PreRenderResult PreRender();
		
		// returns false iff light is obscured by object and should be ignored
		virtual bool GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const;
		
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
		Layer const _layer;
		Program const * _program;
		VboResource const * _vbo_resource;
		bool const _casts_shadow;
	};
}

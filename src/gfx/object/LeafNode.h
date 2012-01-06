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
	// forward-declaration
	class Renderer;
	
	// An element of the scene which can be drawn.
	class LeafNode : public Object
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		enum PreRenderResult
		{
			ok,
			remove
		};
		
		typedef ::Transformation<float> Transformation;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions

		LeafNode(Layer::type layer);
		
		void SetModelViewTransformation(Transformation const & model_view_transformation);
		Transformation const & GetModelViewTransformation() const;
		friend bool operator < (LeafNode const & lhs, LeafNode const & rhs);
		
		Layer::type GetLayer() const;
		
		bool IsOpaque() const;
		void SetIsOpaque(bool is_opaque);
		
		// Return the necessary z-clipping range required to render this object through the given camera.
		virtual bool GetRenderRange(RenderRange & range) const;
		
		// Perform any necessary preparation for rendering.
		virtual PreRenderResult PreRender(Renderer const & renderer);
		
		// Draw the object.
		virtual void Render(Renderer const & renderer) const;

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		// The resultant transformation.
		Transformation _model_view_transformation;
		
		// This is the list which is sorted in order of 
		DEFINE_INTRUSIVE_LIST(LeafNode, RenderList);
		
		float _render_sort_key;
		Layer::type _layer;
		bool _is_opaque;
	};
}

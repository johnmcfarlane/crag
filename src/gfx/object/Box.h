//
//  Box.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"

#include "gfx/Color.h"


namespace gfx
{
	// Box class
	class Box final : public LeafNode
	{
	public:
		// functions
		DECLARE_ALLOCATOR(Box);

		Box(Init const & init, Transformation const & local_transformation, Vector3 const & dimensions, Color4f const & color);

		bool GetRenderRange(RenderRange & range) const override;
		void Render(Engine const & renderer) const override;
		
	private:
		void UpdateModelViewTransformation(Transformation const & model_view) final;
		void GenerateShadowVolume(Light const & light, ShadowVolume & shadow_volume) const final;

		// variables
		Color4f _color;
		Vector3 _dimensions;
	};
}

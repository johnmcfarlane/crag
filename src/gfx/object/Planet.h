//
//  gfx/object/Planet.h
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"


namespace gfx
{
	// graphical representation of a Planet
	class Planet : public LeafNode
	{
	public:
		// types
		class UpdateParams
		{
		public:
			Scalar _radius_min, _radius_max;
		};
		
		// functions
		Planet(Scalar sea_level);
		
		void Update(UpdateParams const & params, Renderer & renderer) override;

	private:
		gfx::Transformation const & Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override;

		bool GetRenderRange(RenderRange & range) const override;
		void Render(Renderer const & renderer) const override;
		
		// variables
		Scalar _sea_level;
		UpdateParams _salient;
	};
}


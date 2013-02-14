//
//  gfx/object/Planet.h
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/LeafNode.h"

namespace gfx
{
	// graphical representation of a Planet
	class Planet : public LeafNode
	{
	public:
		// functions
		DECLARE_ALLOCATOR(Planet);

		Planet(Init const & init, Scalar radius);
		
		void SetRadiusMinMax(Scalar radius_min, Scalar radius_max);

	private:
		virtual Transformation const & Transform(Transformation const & model_view, Transformation & scratch) const final;

		bool GetRenderRange(RenderRange & range) const override;
		void Render(Engine const & renderer) const override;
		
		// variables
		Scalar _sea_level;
		Scalar _radius_min;
		Scalar _radius_max;
	};
}

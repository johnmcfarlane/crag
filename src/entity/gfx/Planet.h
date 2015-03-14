//
//  gfx/object/Planet.h
//  crag
//
//  Created by John McFarlane on 8/30/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/Object.h"

namespace gfx
{
	// graphical representation of a Planet
	class Planet : public Object
	{
	public:
		// functions
		DECLARE_ALLOCATOR(Planet);

		Planet(Engine & engine, Transformation const & local_transformation);
		
		void SetMaxRadius(Scalar max_radius);

	private:
		virtual void UpdateModelViewTransformation(Transformation const & model_view) final;

		bool GetRenderRange(RenderRange & range) const override;
		void Render(Engine const & renderer) const override;

		// variables
		Scalar _max_radius;
	};
}

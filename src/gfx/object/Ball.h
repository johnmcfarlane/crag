//
//  gfx/object/Ball.h
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/LeafNode.h"

#include "gfx/Color.h"

#include "geom/Matrix33.h"


namespace gfx
{
	DECLARE_CLASS_HANDLE(Ball); // gfx::BallHandle

	// Ball class
	class Ball : public LeafNode
	{
	public:
		// functions
		DECLARE_ALLOCATOR(Ball);

		Ball(Init const & init, Transformation const & local_transformation, Color4f const & color);

	private:
		virtual void UpdateModelViewTransformation(Transformation const & model_view_transformation) final;
		
		bool GetRenderRange(RenderRange & range) const override;
		void Render(Engine const & renderer) const override;
		
		// variables
		Color4f _color;
	};
}

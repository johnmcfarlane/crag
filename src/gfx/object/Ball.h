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
	// Ball class
	class Ball : public LeafNode
	{
	public:
		// functions
		Ball(Color4b color);
		
	private:
		bool GetRenderRange(RenderRange & range) const override;
		void Render(Renderer const & renderer) const override;
		
		// variables
		Color4b _color;
	};
}

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
	class Box : public LeafNode
	{
	public:
		// functions
		Box(Color4b color);
		
		bool GetRenderRange(RenderRange & range) const override;
		void Render(Renderer const & renderer) const override;
		
	private:
		// variables
		Color4b _color;
	};
}

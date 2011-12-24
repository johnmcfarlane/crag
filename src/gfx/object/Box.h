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

#include "geom/Transformation.h"


namespace gfx
{
	// forward-declarations
	class Cuboid;
	
	// Box class
	class Box : public LeafNode
	{
	public:
		// functions
		Box(Color4b color);
		
		void Init(Scene const & scene) override;
		
		virtual bool GetRenderRange(RenderRange & range) const;
		virtual void Render() const;
		
	private:
		// variables
		Cuboid const * _cuboid;
		Color4b _color;
	};
}

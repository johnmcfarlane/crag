//
//  Box.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/LeafNode.h"

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
		Box();
		
		void Init(Scene const & scene) override;
		
		virtual bool GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, Scalar * range) const;
		virtual void Render(Transformation const & transformation, Layer::type layer, Pov const & pov) const;
		
	private:
		// variables
		Cuboid const * _cuboid;
	};
}

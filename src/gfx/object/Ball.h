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
	// forward-declarations
	class Sphere;
	
	// Ball class
	class Ball : public LeafNode
	{
	public:
		// functions
		Ball(Color4b color);
		
		void Init(Scene const & scene) override;
		
	private:
		bool GetRenderRange(Transformation const & transformation, Ray3 const & camera_ray, bool wireframe, RenderRange & range) const override;
		void Render() const override;
		
		// variables
		Sphere const * _sphere;
		Color4b _color;
	};
}

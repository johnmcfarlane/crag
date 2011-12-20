//
//  gfx/object/Ball.h
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "LeafNode.h"

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
		Ball();
		
		void Init(Scene const & scene) override;
		
		virtual bool GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, RenderRange & range) const;
		virtual void Render() const;
		
	private:
		unsigned CalculateLod(Scalar radius, Scalar inv_distance_to_camera) const;
		static Scalar CalculateRadius(Transformation const & transformation);
		
		// variables
		Sphere const * _sphere;
		Color4b _color;
	};
}

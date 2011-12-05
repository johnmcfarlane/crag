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
		
		virtual bool GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, Scalar * range) const;
		virtual void Render(Transformation const & transformation, Layer::type layer, Pov const & pov) const;
		
	private:
		unsigned CalculateLod(Scalar radius, Scalar inv_distance_to_camera) const;
		static Scalar CalculateRadius(Transformation const & transformation);
		
		// variables
		Sphere const * _sphere;
	};
}

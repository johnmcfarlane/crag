//
//  Ball.h
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Object.h"

namespace gfx
{
	// forward-declarations
	class Pov;
	
	// Ball class
	class Ball : public Object
	{
	public:
		Ball(Scalar radius);
		
		virtual bool GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const;
		virtual void Render(Layer::type layer, Scene const & scene) const;
		virtual bool IsInLayer(Layer::type) const;
		
		Scalar _radius;
		
	private:
		void SetMatrix(Pov const & pov) const;
		unsigned CalculateLod(Pov const & pov) const;		
	};
}

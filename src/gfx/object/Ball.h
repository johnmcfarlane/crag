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
	class Pov;
	
	// Ball class
	class Ball : public LeafNode
	{
	public:
		// types
		struct UpdateParams
		{
			Vector _position;
			Matrix33 _rotation;
		};
		
		// functions
		Ball(Scalar radius);
		
		virtual bool GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const;
		virtual void Update(UpdateParams const & params);
		virtual void Render(Layer::type layer, Scene const & scene) const;
		
	private:
		unsigned CalculateLod(Vector const & camera_position) const;		
		
		// variables
		Vector _position;
		Matrix33 _rotation;
		Scalar _radius;
	};
}

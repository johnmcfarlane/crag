//
//  Box.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "gfx/object/Object.h"

namespace gfx
{
	// forward-declarations
	class Pov;
	
	// Box class
	class Box : public Object
	{
	public:
		// types
		struct UpdateParams
		{
			Transformation transformation;
		};
		
		// functions
		Box(Vector const & size);
		
		virtual bool GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const;
		virtual void Update(UpdateParams const & params);
		virtual void Render(Layer::type layer, Scene const & scene) const;
		virtual bool IsInLayer(Layer::type) const;
		
	private:
		// variables
		Transformation _transformation;
	};
}

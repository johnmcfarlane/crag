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
		// types
		struct UpdateParams
		{
			Transformation transformation;
		};
		
		// functions
		Box(Vector const & size);
		
		void Init(Scene const & scene) override;
		
		virtual bool GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const;
		virtual void Update(UpdateParams const & params);
		virtual void Render(Layer::type layer, Pov const & pov) const;
		
	private:
		// variables
		Transformation _transformation;
		Cuboid const * _cuboid;
	};
}

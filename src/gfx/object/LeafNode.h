//
//  LeafNode.h
//  crag
//
//  Created by John McFarlane on 2011-11-23.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Object.h"


namespace gfx
{
	class LeafNode : public Object
	{
	public:
		LeafNode(Layer::type layer);
		LeafNode(Layer::Map::type layers);
		
		// Return the necessary z-clipping range required to render this object through the given camera.
		virtual bool GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const;
		
		// Perform any necessary preparation for rendering.
		virtual void PreRender();
		
		// Draw the object.
		virtual void Render(Layer::type layer, Pov const & pov) const;
		
	};
}

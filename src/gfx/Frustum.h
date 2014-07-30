//
//  Frustum.h
//  crag
//
//  Created by John on 2014-07-23.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

namespace gfx
{
	// characteristics about the camera - other than its position;
	// the stuff required to make a projection matrix
	class Frustum
	{
	public:
		Frustum();
		Frustum(geom::Vector2i _resolution, RenderRange _depth_range, Scalar _fov);
		
		Matrix44 CalcProjectionMatrix() const;
		
		Vector2 PixelToScreen(Vector2 const & pixel_position) const;
		Vector2 ScreenToPixel(Vector2 const & screen_position) const;

		geom::Vector2i resolution;
		RenderRange depth_range;	// does this really belong here?
		Scalar fov;	// radians
	};
}

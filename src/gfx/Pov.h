//
//  Pov.h
//  crag
//
//  Created by John on 12/8/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "geom/Transformation.h"

namespace gfx
{
	class Frustum
	{
	public:
		Frustum();
		
		// TODO: Move this to a utils file
		Matrix44 CalcProjectionMatrix() const;
		
		Vector2 PixelToScreen(Vector2 const & pixel_position) const;
		Vector2 ScreenToPixel(Vector2 const & screen_position) const;

		geom::Vector2i resolution;
		RenderRange depth_range;	// does this really belong here?
		float fov;	// radians
	};
	
	// Point of view
	class Pov
	{
	public:
		// functions
		Pov();
		
		void SetFrustum(Frustum const & frustum);
		Frustum const & GetFrustum() const;

		// set/get the camera position/rotation
		void SetTransformation(Transformation const & transformation);
		Transformation const & GetTransformation() const;
		
		Vector3 GetPosition() const;

		Vector2 WorldToPixel(Vector3 const & world_position) const;
		Vector3 PixelToWorld(Vector2 const & pixel_position) const;

		Vector2 WorldToScreen(Vector3 const & world_position) const;
		Vector3 ScreenToWorld(Vector2 const & screen_position) const;
		
	private:
		// variables
		Frustum _frustum;
		Transformation _transformation;
	};
}

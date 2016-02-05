//
//  Pov.cpp
//  crag
//
//  Created by John on 1/7/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Frustum.h"

#include "core/ConfigEntry.h"

#include "geom/Matrix44.h"

#include "core/app.h"
// TODO: Frustum should not depend on app.h.

using namespace gfx;

CONFIG_DEFINE(frustum_default_depth_near, .25f);
CONFIG_DEFINE(frustum_default_depth_far, 1000.f);
CONFIG_DEFINE_ANGLE(frustum_default_fov, 55.f);

////////////////////////////////////////////////////////////////////////////////
// gfx::Frustum member definitions

Frustum::Frustum()
: resolution(app::GetResolution())	// TODO: Frustum should be cheaper to construct.
, depth_range(frustum_default_depth_near, frustum_default_depth_far)
, fov(frustum_default_fov)
{
}

Frustum::Frustum(geom::Vector2i _resolution, RenderRange _depth_range, Scalar _fov)
: resolution(_resolution)	// TODO: Frustum should be cheaper to construct.
, depth_range(_depth_range)
, fov(_fov)
{
}

// This matrix is ready-transposed for OpenGL.
Matrix44 Frustum::CalcProjectionMatrix() const
{
	double aspect = static_cast<double>(resolution.x) / resolution.y;
	double f = 1. / tan(fov * .5);
	return Matrix44(static_cast<float>(f / aspect), 0, 0, 0, 
						0, static_cast<float>(f), 0, 0, 
						0, 0, static_cast<float>((depth_range[1] + depth_range[0]) / (depth_range[0] - depth_range[1])), static_cast<float>(2. * depth_range[1] * depth_range[0] / (depth_range[0] - depth_range[1])),
						0, 0, -1, 0);
}

Vector2 Frustum::PixelToScreen(Vector2 const & pixel_position) const
{
	auto _resolution = static_cast<Vector2>(resolution);

	Vector2 screen_position(
		(2.f * pixel_position.x / _resolution.x) - 1.f,
		(- 2.f * pixel_position.y / _resolution.y) + 1.f);
	
	return screen_position;
}

Vector2 Frustum::ScreenToPixel(Vector2 const & screen_position) const
{
	auto _resolution = static_cast<Vector2>(resolution);

	Vector2 pixel_position(
		(screen_position.x + 1.f) * .5f * _resolution.x,
		(screen_position.y - 1.f) * - .5f * _resolution.y);
	
	return screen_position;
}

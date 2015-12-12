//
//  Pov.cpp
//  crag
//
//  Created by John on 1/7/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "axes.h"
#include "Pov.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Pov member definitions

Pov::Pov()
: _transformation(Transformation::Matrix44::Identity())
{
}

Pov::Pov(Transformation const & transformation, Frustum const & frustum)
: _transformation(transformation)
, _frustum(frustum)
{
}

void Pov::SetFrustum(Frustum const & frustum)
{
	_frustum = frustum;
}

Frustum const & Pov::GetFrustum() const
{
	return _frustum;
}

Frustum & Pov::GetFrustum()
{
	return _frustum;
}

void Pov::SetTransformation(Transformation const & transformation)
{
	_transformation = transformation;
	CRAG_VERIFY(_transformation);
}

Transformation const & Pov::GetTransformation() const
{
	return _transformation;
}

Transformation & Pov::GetTransformation()
{
	return _transformation;
}

Vector3 Pov::GetPosition() const
{
	return _transformation.GetTranslation();
}

Vector2 Pov::WorldToPixel(Vector3 const & world_position) const
{
	auto screen_position = WorldToScreen(world_position);
	auto pixel_position = _frustum.ScreenToPixel(screen_position);
	return pixel_position;
}

Vector3 Pov::PixelToWorld(Vector2 const & pixel_position) const
{
	auto screen_position = _frustum.PixelToScreen(pixel_position);
	auto world_position = ScreenToWorld(screen_position);
	return world_position;
}

Vector2 Pov::WorldToScreen(Vector3 const & world_position) const
{
	Vector4 input(world_position.x, world_position.y, world_position.z, 1.f);
	
	Matrix44 model_view = ToOpenGl(Inverse(_transformation.GetMatrix()));
	Vector4 camera = model_view * input;

	Matrix44 projection = _frustum.CalcProjectionMatrix();
	Vector4 clip = projection * camera;
	
	auto screen_position = Vector2(clip.x, clip.y) / clip.w;
	return screen_position;
}

Vector3 Pov::ScreenToWorld(Vector2 const & screen_position) const
{
	auto resolution = static_cast<Vector2>(_frustum.resolution);

	Vector3 normalized(
		2.f * (screen_position.x / resolution.x) - 1.f,
		- 2.f * (screen_position.y / resolution.y) + 1.f,
		- 1.f);
		
	Vector4 clip(
		screen_position.x,
		screen_position.y,
		-1.f,
		1.f);
		
	auto projection_matrix = Inverse(_frustum.CalcProjectionMatrix());
	auto camera = projection_matrix * clip;
	
	auto model_view = Inverse(ToOpenGl(Inverse(_transformation.GetMatrix())));
	auto input = model_view * camera;
	
	Vector3 world_position(input.x, input.y, input.z);

	return world_position;
}

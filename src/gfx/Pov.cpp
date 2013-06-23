//
//  Pov.cpp
//  crag
//
//  Created by John on 1/7/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Pov.h"

#include "axes.h"

using namespace gfx;
using namespace std;

////////////////////////////////////////////////////////////////////////////////
// gfx::Frustum member definitions

Frustum::Frustum()
: resolution(-1, -1)
, depth_range(-1, -1)
, fov(-1)
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

////////////////////////////////////////////////////////////////////////////////
// gfx::Pov member definitions

Pov::Pov()
: _transformation(Transformation::Matrix44::Identity())
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

void Pov::SetTransformation(Transformation const & transformation)
{
	_transformation = transformation;
	VerifyObject(_transformation);
}

Transformation const & Pov::GetTransformation() const
{
	return _transformation;
}

Vector3 Pov::GetPosition() const
{
	return _transformation.GetTranslation();
}

Vector2 Pov::WorldToScreen(Vector3 const & world_position) const
{
	Vector4 input(world_position.x, world_position.y, world_position.z, 1.f);
	
	Matrix44 model_view = ToOpenGl(Inverse(_transformation.GetMatrix()));
	Vector4 camera = model_view * input;

	Matrix44 projection = _frustum.CalcProjectionMatrix();
	Vector4 clip = projection * camera;
	
	auto normalized = Vector2(clip.x, clip.y) / clip.w;
	
	auto resolution = geom::Cast<float>(_frustum.resolution);
	Vector2 screen_position(
		((normalized.x - 1.f) * -.5f) * resolution.x,
		((normalized.y + 1.f) * .5f) * resolution.y);
		// (normalized.z + 1.f) * .5f
		
	return screen_position;
}

Vector3 Pov::ScreenToWorld(Vector2 const & screen_position) const
{
	auto resolution = geom::Cast<float>(_frustum.resolution);

	Vector3 normalized(
		- 2. * (screen_position.x / resolution.x) + 1.,
		2. * (screen_position.y / resolution.y) - 1.,
		- 1.);
		
	Vector4 clip(
		normalized.x,
		normalized.y,
		normalized.z,
		1);
		
	auto projection_matrix = Inverse(_frustum.CalcProjectionMatrix());
	auto camera = projection_matrix * clip;
	
	auto model_view = Inverse(ToOpenGl(Inverse(_transformation.GetMatrix())));
	auto input = model_view * camera;
	
	Vector3 world_position(input.x, input.y, input.z);

	return world_position;
}

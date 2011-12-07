//
//  gfx/object/Ball.cpp
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Ball.h"

#include "gfx/Scene.h"
#include "gfx/Sphere.h"

#include "glpp/glpp.h"

#include "geom/Transformation.h"


using namespace gfx;


Ball::Ball()
: LeafNode(Layer::foreground)
, _sphere(nullptr)
{
}

void Ball::Init(Scene const & scene)
{
	_sphere = & scene.GetSphere();
}

bool Ball::GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, Scalar * range) const 
{ 
	Scalar distance;
	{
		Vector position = transformation.GetTranslation();
		distance = Distance(camera_ray.position, position);
	}
	
	Scalar radius = CalculateRadius(transformation);
	
	range[0] = distance - radius;
	range[1] = distance + radius;
	
	return true;
}

void Ball::Render(Transformation const & transformation, Layer::type layer) const
{
	GLPP_VERIFY;
	
	// Calculate the LoD.
	Scalar radius = CalculateRadius(transformation);
	Vector relative_position = transformation.GetTranslation();
	Scalar inv_distance = InvSqrt(LengthSq(relative_position));
	unsigned lod = CalculateLod(radius, inv_distance);
	
	// Set the matrix.
	Pov::SetModelViewMatrix(transformation);
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	_sphere->Draw(lod);
	
	GLPP_VERIFY;
}

unsigned Ball::CalculateLod(Scalar radius, Scalar inv_distance_to_camera) const
{
	Scalar mn1 = 1500;
	int lod = int(pow(mn1 * radius * inv_distance_to_camera, .3));
	Clamp(lod, 1, 5);
	-- lod;
	
	return lod;
}

Scalar Ball::CalculateRadius(Transformation const & transformation)
{
	Vector size = transformation.GetScale();
	Scalar radius = Length(size) * .5;
	return radius;
}

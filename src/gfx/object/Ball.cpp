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


Ball::Ball(Scalar radius)
: LeafNode(Layer::foreground)
, _position(Vector::Zero())
, _rotation(Matrix33::Identity()) 
, _radius(radius)
, _sphere(nullptr)
{
}

void Ball::Init(Scene const & scene)
{
	_sphere = & scene.GetSphere();
}

bool Ball::GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const 
{ 
	Scalar distance = Distance(camera_ray.position, _position);
	
	range[0] = distance - _radius;
	range[1] = distance + _radius;
	
	return true;
}

void Ball::Update(UpdateParams const & params)
{
	_position = params._position;
	_rotation = params._rotation;
}

void Ball::Render(Layer::type layer, Pov const & pov) const
{
	GLPP_VERIFY;
	
	// Calculate the LoD.
	unsigned lod = CalculateLod(pov.GetPosition());
	
	// Set the matrix.
	pov.SetModelView(Transformation(_position, _rotation, _radius));
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	_sphere->Draw(lod);
	
	GLPP_VERIFY;
}

unsigned Ball::CalculateLod(Vector const & camera_position) const
{
	Vector relative_position = camera_position - _position;
	
	Scalar mn1 = 1500;
	Scalar inv_distance = InvSqrt(LengthSq(relative_position));
	int lod = int(Power(mn1 * _radius * inv_distance, .3));
	Clamp(lod, 1, 5);
	-- lod;
	
	return lod;
}

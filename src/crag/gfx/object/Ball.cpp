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


using namespace gfx;


Ball::Ball(Scalar radius)
: _position(Vector::Zero())
, _rotation(Matrix::Identity()) 
, _radius(radius)
{
}

bool Ball::GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const 
{ 
	Scalar distance = Length(camera_ray.position - _position);
	
	range[0] = distance - _radius;
	range[1] = distance + _radius;
	
	return true;
}

void Ball::Update(UpdateParams const & params)
{
	_position = params._position;
	_rotation = params._rotation;
}

void Ball::Render(Layer::type layer, gfx::Scene const & scene) const
{
	GLPP_VERIFY;
	
	gfx::Pov const & pov = scene.GetPov();
	
	// Calculate the LoD.
	unsigned lod = CalculateLod(pov);
	
	// Set the matrix.
	pov.SetModelView(_position, _rotation);
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	gfx::Sphere const & sphere = scene.GetSphere();
	sphere.Draw(float(_radius), lod);
	
	GLPP_VERIFY;
}

bool Ball::IsInLayer(Layer::type layer) const 
{ 
	return layer == Layer::foreground; 
}

unsigned Ball::CalculateLod(gfx::Pov const & pov) const
{
	Vector relative_position = pov.pos - _position;
	
	Scalar mn1 = 1500;
	Scalar inv_distance = InvSqrt(LengthSq(relative_position));
	int lod = int(Power(mn1 * _radius * inv_distance, .3));
	Clamp(lod, 1, 5);
	-- lod;
	
	return lod;
}

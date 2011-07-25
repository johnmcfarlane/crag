//
//  Ball.cpp
//  crag
//
//  Created by John McFarlane on 7/17/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Ball.h"

#include "Scene.h"
#include "Sphere.h"

#include "glpp/glpp.h"


using namespace gfx;


Ball::Ball(Scalar radius)
: _radius(radius)
{
}

bool Ball::GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const 
{ 
	Scalar distance = Length(camera_ray.position - GetPosition());
	
	range[0] = distance - _radius;
	range[1] = distance + _radius;
	
	return true;
}

void Ball::Draw(gfx::Scene const & scene) const
{
	GLPP_VERIFY;
	
	gfx::Pov const & pov = scene.GetPov();
	
	// Calculate the LoD.
	unsigned lod = CalculateLod(pov);
	
	// Set the matrix.
	SetMatrix(pov);
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	gfx::Sphere const & sphere = scene.GetSphere();
	sphere.Draw(_radius, lod);
	
	GLPP_VERIFY;
}

RenderStage::type Ball::GetRenderStage() const 
{ 
	return RenderStage::foreground; 
}

void Ball::SetMatrix(gfx::Pov const & pov) const
{
	gfx::Pov entity_pov (pov);
	entity_pov.pos = pov.pos - GetPosition();
	Matrix model_view_matrix = entity_pov.CalcModelViewMatrix();
	model_view_matrix = _rotation * model_view_matrix;
	
	gl::MatrixMode(GL_MODELVIEW);
	gl::LoadMatrix(model_view_matrix.GetArray());	
}

unsigned Ball::CalculateLod(gfx::Pov const & pov) const
{
	Vector relative_position = pov.pos - GetPosition();
	
	Scalar mn1 = 1500;
	Scalar inv_distance = InvSqrt(LengthSq(relative_position));
	int lod = int(Power(mn1 * _radius * inv_distance, .3));
	Clamp(lod, 1, 5);
	-- lod;
	
	return lod;
}

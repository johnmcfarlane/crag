//
//  Box.cpp
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Box.h"

#include "gfx/Scene.h"
#include "gfx/Cuboid.h"

#include "glpp/glpp.h"


using namespace gfx;


Box::Box(Vector const & size)
: _position(Vector::Zero())
, _rotation(Matrix::Identity()) 
, _size(size)
{
}

bool Box::GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const 
{ 
	Scalar distance = Length(camera_ray.position - _position);
	
	// This could be improved by sampling each of the 8 corners of the box
	// but it probably isn't worth the clock cycles to do that.
	float radius = float(Length(_size) * .5);
	range[0] = distance - radius;
	range[1] = distance + radius;
	
	return true;
}

void Box::Update(UpdateParams const & params)
{
	_position = params._position;
	_rotation = params._rotation;
}

void Box::Render(Layer::type layer, gfx::Scene const & scene) const
{
	GLPP_VERIFY;
	
	gfx::Pov const & pov = scene.GetPov();
	
	// Set the matrix.
	pov.SetModelView(_position, _rotation);
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	gfx::Cuboid const & cuboid = scene.GetCuboid();
	cuboid.Draw(_size);
	
	GLPP_VERIFY;
}

bool Box::IsInLayer(Layer::type layer) const 
{ 
	return layer == Layer::foreground; 
}

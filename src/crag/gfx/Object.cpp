//
//  Object.cpp
//  crag
//
//  Created by John McFarlane on 7/23/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Object.h"


using namespace gfx;


Object::Object(Vector const & position, Matrix const & rotation) 
: _position(Vector::Zero())
, _rotation(Matrix::Identity()) 
{ 
}

Object::~Object() 
{ 
}

void Object::Init()
{
}

Object::Vector const & Object::GetPosition() const 
{ 
	return _position; 
}

bool Object::GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const 
{ 
	return false; 
}

void Object::Draw(Scene const & scene) const 
{ 
}

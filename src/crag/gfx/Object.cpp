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


Object::Object()
{ 
}

Object::~Object() 
{ 
}

void Object::Init()
{
}

void Object::Deinit()
{
}

bool Object::GetRenderRange(Ray const & camera_ray, Scalar * range, bool wireframe) const 
{ 
	return false; 
}

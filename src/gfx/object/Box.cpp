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


Box::Box()
: LeafNode(Layer::foreground)
, _cuboid(nullptr)
{
}

void Box::Init(Scene const & scene)
{
	_cuboid = & scene.GetCuboid();
}

bool Box::GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, Scalar * range) const 
{ 
	// This could be improved by sampling each of the 8 corners of the box
	// but it probably isn't worth the clock cycles to do that.
	float radius;
	{
		Vector size = transformation.GetScale();
		radius = float(Length(size) * .5);
	}

	Scalar distance;
	{
		Vector center = transformation.GetTranslation();
		distance = Distance(camera_ray.position, center);
	}
	
	range[0] = distance - radius;
	range[1] = distance + radius;
	
	return true;
}

void Box::Render(Transformation const & transformation, Layer::type layer, Pov const & pov) const
{
	GLPP_VERIFY;
	
	// Set the matrix.
	Pov::SetModelViewMatrix(transformation);
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	_cuboid->Draw();
	
	GLPP_VERIFY;
}

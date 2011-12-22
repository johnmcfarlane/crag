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

#include "core/Random.h"


using namespace gfx;


Box::Box(Color4b color)
: LeafNode(Layer::foreground)
, _cuboid(nullptr)
, _color(color)
{
	SetIsOpaque(_color.a == 255);
}

void Box::Init(Scene const & scene)
{
	_cuboid = & scene.GetCuboid();
}

bool Box::GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, RenderRange & range) const 
{ 
	// This could be improved by sampling each of the 8 corners of the box
	// but it probably isn't worth the clock cycles to do that.
	float radius;
	{
		Vector3 size = transformation.GetScale();
		radius = float(Length(size) * .5);
	}

	Scalar distance;
	{
		Vector3 center = transformation.GetTranslation();
		distance = Distance(camera_ray.position, center);
	}
	
	range[0] = distance - radius;
	range[1] = distance + radius;
	
	return true;
}

void Box::Render() const
{
	GLPP_VERIFY;
	
	gl::SetColor(_color.GetArray());
	
	// Low-LoD meshes are smaller than the sphere they approximate.
	// Apply a corrective scale to compensate.
	_cuboid->Draw();
	
	GLPP_VERIFY;
}

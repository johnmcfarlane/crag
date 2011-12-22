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

#include "core/Random.h"


using namespace gfx;


Ball::Ball(Color4b color)
: LeafNode(Layer::foreground)
, _sphere(nullptr)
, _color(color)
{
	SetIsOpaque(_color.a == 255);
}

void Ball::Init(Scene const & scene)
{
	_sphere = & scene.GetSphere();
}

bool Ball::GetRenderRange(Transformation const & transformation, Ray const & camera_ray, bool wireframe, RenderRange & range) const 
{ 
	Scalar distance;
	{
		Vector3 position = transformation.GetTranslation();
		distance = Distance(camera_ray.position, position);
	}
	
	Scalar radius = Sphere::CalculateRadius(transformation);
	
	range[0] = distance - radius;
	range[1] = distance + radius;
	
	return true;
}

void Ball::Render() const
{
	gl::SetColor(_color.GetArray());
	
	Transformation const & transformation = GetModelViewTransformation();
	_sphere->Draw(transformation);
}

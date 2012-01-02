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
#include "gfx/SphereMesh.h"

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

bool Ball::GetRenderRange(RenderRange & range) const 
{ 
	Transformation const & transformation = GetModelViewTransformation();
	Transformation::Matrix const & transformation_matrix = transformation.GetMatrix();
	Scalar depth = transformation_matrix[1][3];

	Scalar radius = SphereMesh::CalculateRadius(transformation);
	
	range[0] = depth - radius;
	range[1] = depth + radius;
	
	return true;
}

void Ball::Render() const
{
	gl::SetColor(_color.GetArray());
	
	Transformation const & transformation = GetModelViewTransformation();
	_sphere->Draw(transformation);
}

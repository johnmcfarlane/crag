//
//  Pov.cpp
//  crag
//
//  Created by John on 1/7/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Pov.h"

#include "geom/MatrixOps.h"
#include "geom/Sphere.h"

#include "glpp/glpp.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Frustum member definitions

Frustum::Frustum()
: resolution(-1, -1)
, fov(-1)
, depth_range(-1, -1)
{
}

// This matrix is ready-transposed for OpenGL.
Matrix44 Frustum::CalcProjectionMatrix() const
{
	double aspect = static_cast<double>(resolution.x) / resolution.y;
	double f = 1. / tan(fov * .5);
	return Matrix44(static_cast<float>(f / aspect), 0, 0, 0, 
						0, static_cast<float>(f), 0, 0, 
						0, 0, static_cast<float>((depth_range[1] + depth_range[0]) / (depth_range[0] - depth_range[1])), -1,
						0, 0, static_cast<float>(2. * depth_range[1] * depth_range[0] / (depth_range[0] - depth_range[1])), 0);
}

void Frustum::SetProjectionMatrix() const
{
	Matrix44 projection_matrix = CalcProjectionMatrix();
	
	gl::MatrixMode(GL_PROJECTION);
	gl::LoadMatrix(projection_matrix.GetArray());
	gl::MatrixMode(GL_MODELVIEW);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Pov member definitions

Pov::Pov()
{
}

Frustum & Pov::GetFrustum()
{
	return _frustum;
}

Frustum const & Pov::GetFrustum() const
{
	return _frustum;
}

void Pov::SetTransformation(gfx::Transformation const & transformation)
{
	_transformation = transformation;
	VerifyObject(_transformation);
}

gfx::Transformation const & Pov::GetTransformation() const
{
	return _transformation;
}

gfx::Vector3 Pov::GetPosition() const
{
	return _transformation.GetTranslation();
}

#if defined(NOT_DEPRECATED)
// handy for shadows
void Pov::LookAtSphere(Vector const & eye, Sphere3 const & sphere, Vector const & up)
{
	pos = eye;
	
	Vector observer_to_center = sphere.center - pos;
	Vector forward = Normalized(observer_to_center);
	rot = Transposition(DirectionMatrix(forward, up));
	
	Scalar distance = Length(observer_to_center);	// hypotenuse
	Scalar adjacent = sqrt(Square(distance) - Square(sphere.radius));
	Scalar angle = Atan2(sphere.radius, adjacent);
	frustum.fov = 2. * angle;
	
	frustum.depth_range.x = distance - sphere.radius;
	frustum.depth_range.y = distance + sphere.radius;
}
#endif

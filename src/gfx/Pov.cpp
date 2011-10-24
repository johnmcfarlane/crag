/*
 *  Pov.cpp
 *  Crag
 *
 *  Created by John on 1/7/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Pov.h"

#include "sim/axes.h"

#include "geom/MatrixOps.h"
#include "geom/Sphere.h"

#include "glpp/glpp.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Frustum member definitions

// This matrix is ready-transposed for OpenGL.
sim::Matrix44 Frustum::CalcProjectionMatrix() const
{
	double aspect = static_cast<double>(resolution.x) / resolution.y;
	double f = 1. / Tan(fov * .5);
	return sim::Matrix44(static_cast<float>(f / aspect), 0, 0, 0, 
						0, static_cast<float>(f), 0, 0, 
						0, 0, static_cast<float>((far_z + near_z) / (near_z - far_z)), -1,
						0, 0, static_cast<float>(2. * far_z * near_z / (near_z - far_z)), 0);
}

void Frustum::SetProjectionMatrix() const
{
	gl::Viewport(0, 0, resolution.x, resolution.y);
	
	sim::Matrix44 const & projection_matrix = CalcProjectionMatrix();
	
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

void Pov::SetTransformation(Transformation const & transformation)
{
	_transformation = transformation;
}

Pov::Transformation const & Pov::GetTransformation() const
{
	return _transformation;
}

Pov::Vector Pov::GetPosition() const
{
	return _transformation.GetTranslation();
}

#if defined(NOT_DEPRECATED)
// handy for shadows
void Pov::LookAtSphere(Vector const & eye, sim::Sphere3 const & sphere, Vector const & up)
{
	pos = eye;
	
	Vector observer_to_center = sphere.center - pos;
	Vector forward = Normalized(observer_to_center);
	rot = Transposition(DirectionMatrix(forward, up));
	
	sim::Scalar distance = Length(observer_to_center);	// hypotenuse
	sim::Scalar adjacent = Sqrt(Square(distance) - Square(sphere.radius));
	sim::Scalar angle = Atan2(sphere.radius, adjacent);
	frustum.fov = 2. * angle;
	
	frustum.near_z = distance - sphere.radius;
	frustum.far_z = distance + sphere.radius;
}
#endif

void Pov::SetModelView(Transformation const & model_transformation) const
{
	typedef sim::Matrix44 Matrix;
	
	Matrix camera_transform = Inverse(_transformation.GetMatrix());
	
	Matrix model_view_matrix = camera_transform * model_transformation.GetMatrix();
	
	Matrix gl_model_view_matrix = Transposition(model_view_matrix) * axes::SimToOpenGl<sim::Scalar>();	
	
	Assert(gl::GetMatrixMode() == GL_MODELVIEW);
	gl::LoadMatrix(gl_model_view_matrix.GetArray());	
}

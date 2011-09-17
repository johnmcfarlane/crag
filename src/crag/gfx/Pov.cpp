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
#include "geom/Sphere3.h"

#include "glpp/glpp.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Frustum member definitions

// This matrix is ready-transposed for OpenGL.
sim::Matrix4 Frustum::CalcProjectionMatrix() const
{
	double aspect = static_cast<double>(resolution.x) / resolution.y;
	double f = 1. / Tan(fov * .5);
	return sim::Matrix4(static_cast<float>(f / aspect), 0, 0, 0, 
						0, static_cast<float>(f), 0, 0, 
						0, 0, static_cast<float>((far_z + near_z) / (near_z - far_z)), -1,
						0, 0, static_cast<float>(2. * far_z * near_z / (near_z - far_z)), 0);
}

void Frustum::SetProjectionMatrix() const
{
	gl::Viewport(0, 0, resolution.x, resolution.y);
	
	sim::Matrix4 const & projection_matrix = CalcProjectionMatrix();
	
	gl::MatrixMode(GL_PROJECTION);
	gl::LoadMatrix(projection_matrix.GetArray());
	gl::MatrixMode(GL_MODELVIEW);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Pov member definitions

Pov::Pov()
: pos(Vector::Zero())
, rot(Matrix::Identity())
{
}

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

Pov::Matrix Pov::GetCameraMatrix() const
{
	// Put the position and rotation together into one, simulation-space camera matrix.
	// (For the skybox, we don't want translation.) 
	return TranslationMatrix(pos) * rot;
}

void Pov::SetModelView(Vector const & model_position) const
{
	SetModelView(model_position, Matrix::Identity());
}

void Pov::SetModelView(Vector const & model_position, Matrix const & model_rotation) const
{
	Matrix camera_matrix = GetCameraMatrix();
	Matrix camera_transform = Inverse(camera_matrix);
	
	Matrix model_matrix = TranslationMatrix(model_position) * model_rotation;
	
	Matrix model_view_matrix = camera_transform * model_matrix;
	
	Matrix gl_model_view_matrix = Transpose(model_view_matrix) * axes::SimToOpenGl<sim::Scalar>();	
	
	Assert(gl::GetMatrixMode() == GL_MODELVIEW);
	gl::LoadMatrix(gl_model_view_matrix.GetArray());	
}

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


// This matrix is ready-transposed for OpenGL.
sim::Matrix4 Frustum::CalcProjectionMatrix() const
{
	double aspect = static_cast<double>(resolution.x) / resolution.y;
	double f = 1. / Tan(fov * .5);
	return sim::Matrix4(
		static_cast<float>(f / aspect), 0, 0, 0, 
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
}

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

Pov::Matrix Pov::GetCameraMatrix(bool translation) const
{
	// Put the position and rotation together into one, simulation-space camera matrix.
	// (For the skybox, we don't want translation.) 
	return translation ? TranslationMatrix(pos) * rot : rot;
}

Pov::Matrix Pov::CameraToModelViewMatrix(Matrix const & camera)
{
	// Between the simulation and GL, z and y are swapped and z is negated.
	Matrix gl_camera = camera * axes::SimToOpenGl<sim::Scalar>();
	
	// And we're rotating the universe around the camera. 
	Matrix gl_world = Inverse(gl_camera);
	
	// And finally, OpenGL is column-major, so transpose (same as we do in Frustum::CalcProjectionMatrix).
	return Transpose(gl_world);
}

Pov::Matrix Pov::CalcModelViewMatrix(bool translation) const
{
	// Put the position and rotation together into one, simulation-space camera matrix.
	// (For the skybox, we don't want translation.) 
	Matrix sim_camera = GetCameraMatrix(translation);
	
	return CameraToModelViewMatrix(sim_camera);	
}

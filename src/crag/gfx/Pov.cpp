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

#include "sim/Space.h"

#include "geom/MatrixOps.h"
#include "geom/Sphere3.h"


// This matrix is ready-transposed for OpenGL.
sim::Matrix4 gfx::Frustum::CalcProjectionMatrix() const
{
	double aspect = static_cast<double>(resolution.x) / resolution.y;
	double f = 1. / Tan(fov * .5);
	return sim::Matrix4(
		static_cast<float>(f / aspect), 0, 0, 0, 
		0, static_cast<float>(f), 0, 0, 
		0, 0, static_cast<float>((far_z + near_z) / (near_z - far_z)), -1,
		0, 0, static_cast<float>(2. * far_z * near_z / (near_z - far_z)), 0);
}

void gfx::Pov::LookAtSphere(sim::Vector3 const & eye, sim::Sphere3 const & sphere, sim::Vector3 const & up)
{
	pos = eye;

	sim::Vector3 observer_to_center = sphere.center - pos;
	sim::Vector3 forward = Normalized(observer_to_center);
	rot = Transposition(DirectionMatrix(forward, up));

	sim::Scalar distance = Length(observer_to_center);	// hypotenuse
	sim::Scalar adjacent = Sqrt(Square(distance) - Square(sphere.radius));
	sim::Scalar angle = Atan2(sphere.radius, adjacent);
	frustum.fov = 2. * angle;
	
	frustum.near_z = distance - sphere.radius;
	frustum.far_z = distance + sphere.radius;
}

sim::Matrix4 gfx::Pov::GetCameraMatrix(bool translation) const
{
	// Put the position and rotation together into one, simulation-space camera matrix.
	// (For the skybox, we don't want translation.) 
	return translation ? TranslationMatrix(pos) * rot : rot;
}

sim::Matrix4 gfx::Pov::CameraToModelViewMatrix(sim::Matrix4 const & camera)
{
	// Between the simulation and GL, z and y are swapped and z is negated.
	sim::Matrix4 gl_camera = camera * space::SimToOpenGl<sim::Scalar>();
	
	// And we're rotating the universe around the camera. 
	sim::Matrix4 gl_world = Inverse(gl_camera);
	
	// And finally, OpenGL is column-major, so transpose (same as we do in gfx::Frustum::CalcProjectionMatrix).
	return Transpose(gl_world);
}

sim::Matrix4 gfx::Pov::CalcModelViewMatrix(bool translation) const
{
	// Put the position and rotation together into one, simulation-space camera matrix.
	// (For the skybox, we don't want translation.) 
	sim::Matrix4 sim_camera = GetCameraMatrix(translation);
	
	return CameraToModelViewMatrix(sim_camera);	
}


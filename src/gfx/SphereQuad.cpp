//
//  SphereQuad.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-01.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "SphereQuad.h"

#include "Program.h"

#include "sim/axes.h"

#include "geom/Transformation.h"


using gfx::SphereQuad;


SphereQuad::SphereQuad(gfx::Program & program)
{
	InitProgram(program);
	InitQuad();
}

SphereQuad::~SphereQuad()
{
	gl::DeleteBuffer(_quad);
}

gfx::Scalar SphereQuad::CalculateRadius(gfx::Transformation const & transformation)
{
	Vector3 size = transformation.GetScale();
	Assert(NearEqual(size.x / size.y, 1, 0.0001));
	Assert(NearEqual(size.y / size.z, 1, 0.0001));
	Assert(NearEqual(size.z / size.x, 1, 0.0001));
	Scalar radius = size.x;
	return radius;
}

gfx::Transformation const & SphereQuad::Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	Transformation::Vector translation = model_view.GetTranslation();

	Transformation::Vector camera_to_center = Normalized(translation);
	Transformation::Rotation rotation = Inverse(axes::Rotation(camera_to_center));

	Transformation::Scalar radius = CalculateRadius(model_view);

	scratch = Transformation(translation, rotation, radius);
	
	return scratch;
}

void SphereQuad::Draw(::Transformation<float> const & model_view) const
{
	Vector4f center = Vector4f(0,0,0,1) * model_view.GetOpenGlMatrix();
	gl::Uniform<float, 3>(_center_location, center.GetAxes());

	float radius = static_cast<float>(CalculateRadius(model_view));
	gl::Uniform(_radius_location, radius);

	gl::BindBuffer(_quad);
	_quad.Activate();
	
	_quad.DrawTris(0, 6);
	
	_quad.Deactivate();
	gl::UnbindBuffer(_quad);
	
	GLPP_VERIFY;
}

void SphereQuad::InitProgram(gfx::Program & program)
{
	program.Use();
	
	_center_location = gl::GetUniformLocation(program, "center");
	_radius_location = gl::GetUniformLocation(program, "radius");
	
	program.Disuse();
}

void SphereQuad::InitQuad()
{
	float const y = -1;
	float const xz0 = -1, xz1 = 1;
	
	float const x0 = xz0, x1 = xz1;
	float const z0 = xz0, z1 = xz1;
	
	QuadVertex verts[6] = 
	{
		{ Vector3f(x0, y, z0) },
		{ Vector3f(x0, y, z1) },
		{ Vector3f(x1, y, z0) },
		{ Vector3f(x1, y, z0) },
		{ Vector3f(x0, y, z1) },
		{ Vector3f(x1, y, z1) }
	};
	
	gl::GenBuffer(_quad);
	gl::BindBuffer(_quad);
	BufferData(_quad, 6, verts, gl::STATIC_DRAW);	// !? compiles !?
	gl::UnbindBuffer(_quad);
}

//
//  Quad.cpp
//  crag
//
//  Created by John McFarlane on 2012-01-01.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2011 John McFarlane. All rights reserved.
//

#include "pch.h"

#include "Quad.h"

#include "sim/axes.h"

#include "geom/Transformation.h"


using namespace gfx;


Quad::Quad(float depth_offset)
{
	float const y = depth_offset;
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

Quad::~Quad()
{
	gl::DeleteBuffer(_quad);
}

gfx::Transformation const & Quad::Transform(gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	Transformation::Vector translation = model_view.GetTranslation();

	Transformation::Vector camera_to_center = Normalized(translation);
	Transformation::Rotation rotation = Inverse(axes::Rotation(camera_to_center));

	Transformation::Vector scale = model_view.GetScale();

	scratch = Transformation(translation, rotation, scale);
	
	return scratch;
}

void Quad::Draw() const
{
	// TODO: Sort objects by buffer object to avoid most of the gl calls here.
	gl::BindBuffer(_quad);
	_quad.Activate();
	
	_quad.DrawTris(0, 6);
	
	_quad.Deactivate();
	gl::UnbindBuffer(_quad);
	
	GLPP_VERIFY;
}

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

#include "geom/origin.h"
#include "geom/Transformation.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// vertex helper functions

template <>
void EnableClientState<Quad::Vertex>()
{
	GL_CALL(glEnableVertexAttribArray(1));
}

template <>
void DisableClientState<Quad::Vertex>()
{
	GL_CALL(glDisableVertexAttribArray(1));
}

template <>
void Pointer<Quad::Vertex>()
{
	VertexAttribPointer<1, Quad::Vertex, decltype(Quad::Vertex::pos), & Quad::Vertex::pos>();
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Quad member definitions

Quad::Quad(float depth_offset)
{
	float const y = depth_offset;
	float const xz0 = -1, xz1 = 1;
	
	float const x0 = xz0, x1 = xz1;
	float const z0 = xz0, z1 = xz1;
	
	Vertex verts[6] = 
	{
		{ geom::Vector3f(x0, y, z0) },
		{ geom::Vector3f(x0, y, z1) },
		{ geom::Vector3f(x1, y, z0) },
		{ geom::Vector3f(x1, y, z0) },
		{ geom::Vector3f(x0, y, z1) },
		{ geom::Vector3f(x1, y, z1) }
	};
	
	_quad.Init();
	_quad.Bind();
	_quad.BufferData(6, verts, GL_STATIC_DRAW);	// !? compiles !?
	_quad.Unbind();
}

Quad::~Quad()
{
	_quad.Deinit();
}

Transformation Quad::CalculateModelViewTransformation(Transformation const & leaf) const
{
	Transformation::Vector3 translation = leaf.GetTranslation();

	Transformation::Vector3 camera_to_center = Normalized(translation);
	Transformation::Matrix33 rotation = Inverse(axes::Rotation(camera_to_center));

	Transformation::Vector3 scale = leaf.GetScale();

	return Transformation(translation, rotation, scale);
}

void Quad::Activate() const
{
	_quad.Bind();
	_quad.Activate();
}

void Quad::Deactivate() const
{
	_quad.Deactivate();
	_quad.Unbind();
}

void Quad::Draw() const
{
	_quad.DrawTris(0, 6);
}

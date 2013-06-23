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

#include "axes.h"

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
	float const z = depth_offset;
	float const xy0 = -1, xy1 = 1;
	
	float const x0 = xy0, x1 = xy1;
	float const y0 = xy0, y1 = xy1;
	
	Vertex verts[6] = 
	{
		{ geom::Vector3f(x0, y0, z) },
		{ geom::Vector3f(x0, y1, z) },
		{ geom::Vector3f(x1, y0, z) },
		{ geom::Vector3f(x1, y0, z) },
		{ geom::Vector3f(x0, y1, z) },
		{ geom::Vector3f(x1, y1, z) }
	};
	
	_quad.Init();
	_quad.Bind();
	_quad.BufferData(6, verts, GL_STATIC_DRAW);
	_quad.Unbind();
}

Quad::~Quad()
{
	_quad.Deinit();
}

Transformation Quad::CalculateModelViewTransformation(Transformation const & leaf) const
{
	Transformation::Vector3 translation = leaf.GetTranslation();

	auto distance_to_camera = Length(translation);
	if (distance_to_camera == 0)
	{
		return leaf;
	}
	
	Transformation::Vector3 camera_to_center = translation / distance_to_camera;
	Transformation::Matrix33 rotation = Inverse(gfx::Rotation(camera_to_center));

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

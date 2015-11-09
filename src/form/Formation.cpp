//
//  Formation.cpp
//  crag
//
//  Created by john on 6/2/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "form/Formation.h"
#include "form/Mesh.h"
#include "form/Shader.h"

using namespace form;


Formation::Formation(int seed, ShaderPtr const & shader, geom::uni::Sphere3 const & shape)
: _seed(seed)
, _shader(shader)
, _shape(shape)
, _max_radius(_shape.radius)
{
}

Shader const & Formation::GetShader() const
{
	return * _shader;
}

geom::uni::Sphere3 const & Formation::GetShape() const
{
	return _shape;
}

int Formation::GetSeed() const
{
	return _seed;
}

void Formation::SampleRadius(geom::uni::Scalar sample_radius)
{
	if (sample_radius > _max_radius)
	{
		_max_radius = sample_radius;
	}
}

geom::uni::Scalar Formation::GetMaxRadius() const
{
	return _max_radius;
}

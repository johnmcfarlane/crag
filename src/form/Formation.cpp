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


Formation::Formation(int seed, ShaderPtr const & shader, geom::abs::Sphere3 const & shape)
: _seed(seed)
, _shader(shader)
, _shape(shape)
{
	_radius_min = _shape.radius;
	_radius_max = _shape.radius;
}

Shader const & Formation::GetShader() const
{
	return * _shader;
}

geom::abs::Sphere3 const & Formation::GetShape() const
{
	return _shape;
}

int Formation::GetSeed() const
{
	return _seed;
}

void Formation::SampleRadius(geom::abs::Scalar sample_radius)
{
	if (sample_radius < _radius_min)
	{
		_radius_min = sample_radius;
	}
	if (sample_radius > _radius_max)
	{
		_radius_max = sample_radius;
	}
}

geom::abs::Vector2 Formation::GetRadiusRange() const
{
	return geom::abs::Vector2(_radius_min, _radius_max);
}

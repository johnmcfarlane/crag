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

#include "form/node/Shader.h"
#include "form/scene/Mesh.h"

#include "sim/EntityMessage.h"
#include "sim/Planet.h"


#if defined(GFX_DEBUG)
#define DEBUG_NORMALS 0
#else
#define DEBUG_NORMALS 0
#endif


using namespace form;


namespace 
{
	class SetRadiusFunctor : public sim::EntityMessage<sim::Planet>
	{
		// types
		typedef sim::EntityMessage<sim::Planet> super;
	public:
		
		// functions
		SetRadiusFunctor(sim::Uid uid, sim::Scalar radius_min, sim::Scalar radius_max)
		: super(uid)
		, _radius_min(radius_min)
		, _radius_max(radius_max)
		{
		}
		
	private:
		void operator() (sim::Planet & planet) const
		{
			planet.SetRadiusMinMax(_radius_min, _radius_max);
		}
		
		sim::Scalar _radius_min;
		sim::Scalar _radius_max;
	};
}


Formation::Formation(int seed, Shader const & shader, sim::Sphere3 const & shape, smp::Uid uid)
: _seed(seed)
, _shader(shader)
, _shape(shape)
, _uid(uid)
{
	_radius_min = _shape.radius;
	_radius_max = _shape.radius;
}

Formation::~Formation()
{
	delete & _shader;
}

Shader const & Formation::GetShader() const
{
	return _shader;
}

sim::Sphere3 const & Formation::GetShape() const
{
	return _shape;
}

int Formation::GetSeed() const
{
	return _seed;
}

void Formation::SendRadiusUpdateMessage() const
{
	SetRadiusFunctor message(_uid, _radius_min, _radius_max);
	sim::Daemon::SendMessage(message);
}

void Formation::SampleRadius(sim::Scalar sample_radius)
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

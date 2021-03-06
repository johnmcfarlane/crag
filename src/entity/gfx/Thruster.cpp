//
//  gfx/Thruster.cpp
//  crag
//
//  Created by John McFarlane on 11/19/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Thruster.h"

#include "gfx/Engine.h"
#include "gfx/Scene.h"

#include <geom/utils.h>

#include "core/ConfigEntry.h"
#include "core/Random.h"


using namespace gfx;


namespace
{
	CONFIG_DEFINE(puff_probability, .25f);
	CONFIG_DEFINE(thruster_color, Color4f(1.f, .5f, .25f));
	CONFIG_DEFINE(puff_drift_coefficient, .75f);
	CONFIG_DEFINE(puff_volume_variance, 1.f);
	CONFIG_DEFINE(puff_volume_median, 0.0005f);
	
	LightAttributes const thruster_light_attribs =
	{
		LightResolution::vertex,
		LightType::point,
		false
	};
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Thruster member definitions

Thruster::Thruster(Engine & engine, Transformation const & local_transformation, float thrust_max)
: super(engine, local_transformation, thruster_color, thruster_light_attribs)
, _thrust_max(thrust_max)
{
}

void Thruster::Update(float const & thrust_factor)
{
	_thrust_factor = thrust_factor;
	if (_thrust_factor != 0)
	{
		if (Random::sequence.GetBool(puff_probability))
		{
			AddPuff(_thrust_factor);
		}
	}
}

Object::PreRenderResult Thruster::PreRender()
{
	Light::PreRender();
	
	SetColor(thruster_color * _thrust_factor * Random::sequence.GetFloat<float>());
	
	// TODO: Resetting this here is wrong. (What happens if sim stalls?)
	_thrust_factor = 0;
	
	return ok;
}

void Thruster::AddPuff(float thrust_factor)
{
	// Get some random numbers.
	Scalar g1, g2, g3, g4;
	Random::sequence.GetGaussians<Scalar>(g1, g2);
	Random::sequence.GetGaussians<Scalar>(g3, g4);
		
	// Get the transformation of the thruster 
	auto model_transformation = GetModelTransformation();
		
	// Calculate the thruster/puff direction.
	Vector3 thruster_direction = GetAxis(model_transformation.GetRotation(), geom::Direction::forward);
	thruster_direction *= Scalar(1) / _thrust_max;
	
	// decide how big the puff will be
	Scalar spawn_volume;
	{
		auto thrust = _thrust_max * thrust_factor;

		spawn_volume = exp(g4 * puff_volume_variance) * thrust * puff_volume_median;
	}

	// Determine the position/direction etc. of the puff.
	Transformation transformation;
	{
		Vector3 puff_direction = (thruster_direction * (Scalar(1) - puff_drift_coefficient)) + (Vector3(g1, g2, g3) * puff_drift_coefficient);
		Normalize(puff_direction);

		// Extract the global thruster translation.
		auto translation = model_transformation.GetTranslation();
		
		transformation = Transformation(translation, geom::Rotation<Scalar>(puff_direction));
	}

	ipc::Handle<Puff>::Create(transformation, spawn_volume);
}

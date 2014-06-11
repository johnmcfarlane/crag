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

#include "Puff.h"

#include "gfx/axes.h"
#include "gfx/Engine.h"
#include "gfx/Scene.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"


using namespace gfx;


namespace
{
	CONFIG_DEFINE(puff_probability, float, .25f);
	CONFIG_DEFINE(thruster_color, Color4f, Color4f(1.f, .5f, .25f));
	CONFIG_DEFINE(puff_drift_coefficient, Scalar, .75);
	CONFIG_DEFINE(puff_volume_variance, Scalar, 1.);
	CONFIG_DEFINE(puff_volume_median, Scalar, 0.0005f);
	
	LightAttributes const thruster_light_attribs =
	{
#if defined(CRAG_USE_GL)
		LightResolution::fragment,
		LightType::point,
		true
#endif

#if defined(CRAG_USE_GLES)
		LightResolution::vertex;
		LightType::point,
		false;
#endif
	};
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Thruster member definitions

DEFINE_POOL_ALLOCATOR(Thruster, 64);

Thruster::Thruster(super::Init const & init, Transformation const & local_transformation, float thrust_max)
: super(init, local_transformation, thruster_color, thruster_light_attribs)
, _thrust_max(thrust_max)
{
}

void Thruster::Update(float const & thrust_factor)
{
	_thrust_factor = thrust_factor;
	if (_thrust_factor != 0)
	{
		if (Random::sequence.GetUnit<float>() < puff_probability)
		{
			AddPuff(_thrust_factor);
		}
	}
}

LeafNode::PreRenderResult Thruster::PreRender()
{
	Light::PreRender();
	
	SetColor(thruster_color * _thrust_factor * Random::sequence.GetUnit<float>());
	
	// TODO: Resetting this here is wrong. (What happens if sim stalls?)
	_thrust_factor = 0;
	
	return LeafNode::ok;
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
	Vector3 thruster_direction = GetAxis(model_transformation.GetRotation(), Direction::forward);
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
		
		transformation = Transformation(translation, gfx::Rotation<Scalar>(puff_direction));
	}

	ipc::Handle<Puff>::CreateHandle(transformation, spawn_volume);
}

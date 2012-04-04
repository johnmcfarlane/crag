//
//  Thruster.cpp
//  crag
//
//  Created by John McFarlane on 11/19/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Thruster.h"

#include "Puff.h"

#include "gfx/Engine.h"
#include "gfx/Scene.h"

#include "sim/axes.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"


using namespace gfx;


namespace
{
	CONFIG_DEFINE(puff_probability, float, .25f);
	CONFIG_DEFINE(thruster_color, Color4f, Color4f(1.f, .5f, .25f));
	CONFIG_DEFINE(puff_drift_coefficient, Scalar, .75);
	CONFIG_DEFINE(puff_volume_variance, Scalar, 1.);
	CONFIG_DEFINE(puff_volume_median, Scalar, 0.0005);
}


////////////////////////////////////////////////////////////////////////////////
// gfx::Thruster member definitions

Thruster::Thruster()
: _renderer(nullptr)
, _thrust_factor(0)
{
}

void Thruster::Init(gfx::Engine & renderer)
{
	super::Init(renderer, thruster_color);
	
	_renderer = & renderer;
}

void Thruster::Update(float const & thrust_factor)
{
	_thrust_factor = thrust_factor;
	if (_thrust_factor != 0)
	{
		if (Random::sequence.GetUnit<float>() < puff_probability)
		{
			AddPuff(_thrust_factor, ref(_renderer));
		}
	}
}

LeafNode::PreRenderResult Thruster::PreRender(gfx::Engine const & renderer)
{
	SetColor(thruster_color * _thrust_factor * Random::sequence.GetUnit<float>());
	
	// TODO: Resetting this here is wrong. (What happens if sim stalls?)
	_thrust_factor = 0;
	
	return LeafNode::ok;
}

void Thruster::AddPuff(float thrust_factor, gfx::Engine & renderer)
{
	BranchNode * branch_node = new BranchNode;
	Uid parent_uid;
	{
		// Create BranchNode to store the positional information of the puff.
		renderer.OnAddObject(* branch_node);
		parent_uid = branch_node->GetUid();
	}
	
	// Determine the position/direction etc. of the puff.
	Scalar spawn_volume;
	{
		// Get some random numbers.
		Scalar g1, g2, g3, g4;
		Random::sequence.GetGaussians<Scalar>(g1, g2);
		Random::sequence.GetGaussians<Scalar>(g3, g4);
		
		// Get the transformation of the 
		gfx::Transformation model_transformation = GetParent()->GetModelTransformation();
		
		// Extract the global thruster translation.
		Vector3 translation = model_transformation.GetTranslation();
		
		// Calculate the thruster/puff direction.
		Vector3 thruster_direction = axes::GetAxis(model_transformation.GetRotation(), axes::FORWARD);
		Scalar thruster_max = Length(thruster_direction);
		thruster_direction *= (1. / thruster_max);
		
		Vector3 puff_direction = (thruster_direction * (1. - puff_drift_coefficient)) + (Vector3(g1, g2, g3) * puff_drift_coefficient);
		Normalize(puff_direction);

		Scalar thrust = thruster_max * thrust_factor;
		spawn_volume = exp(g4 * puff_volume_variance) * thrust * puff_volume_median;

		// Set its position.
		gfx::Transformation transformation(translation, axes::Rotation<Scalar>(puff_direction));
		branch_node->SetTransformation(transformation);
	}
	
	Puff * model = new Puff(spawn_volume);
	renderer.OnAddObject(* model);
	model->Init(renderer);
	renderer.OnSetParent(* model, * branch_node);
}

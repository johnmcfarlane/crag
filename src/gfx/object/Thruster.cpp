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

#include "gfx/Renderer.h"
#include "gfx/Scene.h"
#include "gfx/SphereMesh.h"

#include "sim/axes.h"

#include "glpp/glpp.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::Thruster member definitions

Thruster::Thruster()
: Light(Color4f(1.f, .5f, .25f))
, _thrust_factor(0)
{
}

void Thruster::Update(UpdateParams const & params, Renderer & renderer)
{
	_thrust_factor = params.thrust_factor;
	if (_thrust_factor != 0)
	{
		AddPuff(_thrust_factor, renderer);
	}
}

LeafNode::PreRenderResult Thruster::PreRender(Renderer const & renderer)
{
	SetColor(Color4f(1.f, .5f, .25f) * _thrust_factor * Random::sequence.GetUnit<float>());
	_thrust_factor = 0;
	
	return LeafNode::ok;
}

void Thruster::AddPuff(float thrust_factor, Renderer & renderer)
{
	Uid parent_uid;
	{
		// Create BranchNode to store the positional information of the puff.
		Object * branch_node = new BranchNode;
		renderer.OnAddObject(branch_node, Uid::null);
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
		
		static const double puff_drift_coefficient = .75;
		Vector3 puff_direction = (thruster_direction * (1. - puff_drift_coefficient)) + (Vector3(g1, g2, g3) * puff_drift_coefficient);
		Normalize(puff_direction);

		Scalar thrust = thruster_max * thrust_factor;
		spawn_volume = exp(g4 * 1.5) * thrust * 0.00005;

		// Set its position.
		BranchNode::UpdateParams params = 
		{
			gfx::Transformation(translation, axes::Rotation<Scalar>(puff_direction))
		};
		renderer.OnUpdateObject<BranchNode>(parent_uid, params);
	}
	
	Object * model = new Puff(spawn_volume);
	renderer.OnAddObject(model, parent_uid);
}

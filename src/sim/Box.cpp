//
//  Box.cpp
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Box.h"

#include "EntityFunctions.h"
#include "Engine.h"

#include "physics/BoxBody.h"

#include "gfx/object/Box.h"

#include "geom/Transformation.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"


namespace 
{
	////////////////////////////////////////////////////////////////////////////////
	// Config values
	
	CONFIG_DEFINE (box_density, physics::Scalar, 1);
	
	CONFIG_DEFINE (box_linear_damping, physics::Scalar, 0.005f);
	CONFIG_DEFINE (box_angular_damping, physics::Scalar, 0.005f);
}


////////////////////////////////////////////////////////////////////////////////
// sim::Box member definitions

using namespace sim;

DEFINE_POOL_ALLOCATOR(Box, 10);

Box::Box(Entity::Init const & init, Vector3 center, Vector3 size)
: Entity(init)
{
	InitPhysics(init.engine, center, size);
	InitGraphics();
}

Box::~Box()
{
	_model.Destroy();
}

void Box::InitPhysics(sim::Engine & simulation_engine, Vector3 center, Vector3 size)
{
	// physics
	physics::Engine & physics_engine = simulation_engine.GetPhysicsEngine();	
	physics::BoxBody * body = new physics::BoxBody(physics_engine, true, size);
	body->SetPosition(center);
	body->SetDensity(box_density);
	body->SetLinearDamping(box_linear_damping);
	body->SetAngularDamping(box_angular_damping);
	SetBody(body);
}

void Box::InitGraphics()
{
	gfx::Color4f color(Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>(), 
					Random::sequence.GetUnitInclusive<float>());

	_model = AddModelWithTransform<gfx::Box>(color);
	UpdateModels();
}

void Box::UpdateModels() const
{
	physics::BoxBody const * body = static_cast<physics::BoxBody const *>(GetBody());
	if (body == nullptr)
	{
		return;
	}
	
	gfx::Transformation transformation(body->GetPosition(), body->GetRotation(), body->GetDimensions());
	
	_model.Call([transformation] (gfx::BranchNode & node) {
		node.SetTransformation(transformation);
	});
}

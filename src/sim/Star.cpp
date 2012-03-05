//
//  Star.cpp
//  crag
//
//  Created by John on 12/22/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Star.h"

#include "EntityFunctions.h"
#include "Simulation.h"

#include "gfx/object/Light.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Star member definitions

sim::Star::Star()
: Entity()
, radius(-1)
, year(-1)
{
}

sim::Star::~Star()
{
	_model.Destroy();
}

void sim::Star::Init(Simulation & simulation, InitData const & init_data)
{
	radius = init_data.radius;
	year = init_data.year;
	
	// initialize light
	gfx::Color4f color(gfx::Color4f(1.f,.95f,.9f) * 7500000000000000.f);
	_model = AddModelWithTransform<gfx::Light>(color);
}

void sim::Star::Tick(Simulation & simulation)
{
	Time t = simulation.GetTime();
	Scalar angle = static_cast<Scalar>(t * (2. * PI) / year) + 3.6;
	position = Vector3(- sin(angle) * radius, - cos(angle) * radius, static_cast<Scalar>(0));
}

void sim::Star::UpdateModels() const
{
	gfx::Transformation transformation(position);	
	_model.Call(& gfx::BranchNode::SetTransformation, transformation);
}

sim::Scalar sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}

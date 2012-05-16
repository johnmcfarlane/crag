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
#include "Engine.h"

#include "gfx/object/Light.h"


////////////////////////////////////////////////////////////////////////////////
// sim::Star member definitions

sim::Star::Star(Entity::Init const & init, Scalar radius, Scalar year)
: Entity(init)
, _radius(radius)
, _year(year)
{
	// initialize light
	gfx::Color4f color(gfx::Color4f(1.f,.95f,.9f) * 7500000000000000.f);
	color.a = 1.0f;
	_model = AddModelWithTransform<gfx::Light>(color);
}

sim::Star::~Star()
{
	_model.Destroy();
}

void sim::Star::Tick(sim::Engine & simulation_engine)
{
	Time t = simulation_engine.GetTime();
	Scalar angle = static_cast<Scalar>(t * (2. * PI) / _year) + 3.6;
	position = Vector3(- sin(angle) * _radius, - cos(angle) * _radius, static_cast<Scalar>(0));
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

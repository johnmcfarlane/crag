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
	// initialize position to something reasonable
	CalculatePosition(0);
	
	// initialize light
	gfx::Color4f color(gfx::Color4f(1.f,.95f,.9f) * 7500000000000000.f);
	_model = AddModelWithTransform<gfx::Light>(color);
	
	VerifyObject(*this);
}

sim::Star::~Star()
{
	VerifyObject(* this);

	_model.Destroy();
}

void sim::Star::Tick(sim::Engine & simulation_engine)
{
	core::Time t = simulation_engine.GetTime();
	CalculatePosition(t);
}

void sim::Star::UpdateModels() const
{
	gfx::Transformation transformation(position);	
	_model.Call([transformation] (gfx::BranchNode & node) {
		node.SetTransformation(transformation);
	});
}

sim::Scalar sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}

#if defined(VERIFY)
void sim::Star::Verify() const
{
	Entity::Verify();
	
	VerifyTrue(_model);
	VerifyObject(position);
	VerifyEqual(_radius, _radius);
	VerifyEqual(_year, _year);
}
#endif

void sim::Star::CalculatePosition(core::Time t)
{
	Scalar angle = Scalar(t * (2. * PI) / _year) + 3.6;
	position = Vector3(- std::sin(angle) * _radius, - std::cos(angle) * _radius, Scalar(0));
}

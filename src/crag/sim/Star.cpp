/*
 *  Star.cpp
 *  Crag
 *
 *  Created by John on 12/22/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Star.h"
#include "Universe.h"


sim::Star::Star(float init_radius, float init_year)
: light(Vector3f::Zero(), gfx::Color4f(1.0f, 1.00f, 1.0f), 0, 0, 1, true)
, radius(init_radius)
, year(init_year)
{
}

void sim::Star::Tick()
{
	float angle = static_cast<float>(Universe::time * (2. * PI) / year);
	Vector3f p = Vector3f(- Sin(angle) * radius, - Cos(angle) * radius, 0);
	light.SetPosition(p);
}

float sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}

sim::Vector3 const & sim::Star::GetPosition() const
{
	return light.GetPosition();
}

gfx::Light const & sim::Star::GetLight() const
{
	return light;
}

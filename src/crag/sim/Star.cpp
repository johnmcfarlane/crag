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
: light(Vector3f::Zero(), gfx::Color4f(1.0f, 1.0f, 1.0f), 0, 0, 1, true)
, radius(init_radius)
, year(init_year)
{
}

void sim::Star::Tick()
{
	Scalar angle = static_cast<Scalar>(app::TimeTypeToSeconds(Universe::time) * (2. * PI) / year) + 5;
	position = Vector3(- Sin(angle) * radius, - Cos(angle) * radius, 0);
	light.SetPosition(position);
}

sim::Scalar sim::Star::GetBoundingRadius() const
{
	return 0;	// really just a point light for now
}

sim::Vector3 const & sim::Star::GetPosition() const
{
	return position;
}

gfx::Light const & sim::Star::GetLight() const
{
	return light;
}

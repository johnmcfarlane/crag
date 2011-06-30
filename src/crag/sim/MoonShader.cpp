/*
 *  MoonShader.cpp
 *  crag
 *
 *  Created by John on 8/9/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "MoonShader.h"
#include "Planet.h"

#include "form/Formation.h"
#include "form/node/Node.h"
#include "form/node/Point.h"

#include "geom/SphereOps.h"

#include "core/ConfigEntry.h"


namespace 
{
	
	sim::Scalar root_three = Sqrt(3.);
	

	// RootNode initialization data, i.e. a tetrahedron.
	sim::Vector3 root_corners[4] = 
	{
		sim::Vector3(-1,  1,  1),
		sim::Vector3( 1, -1,  1),
		sim::Vector3( 1,  1, -1),
		sim::Vector3(-1, -1, -1)
	};
	
}


////////////////////////////////////////////////////////////////////////////////
// MoonShader

sim::MoonShader::MoonShader(Planet const & init_moon, int num_craters)
: center(Vector3::Zero())
, moon(init_moon)
{
	int seed = moon.GetFormation().seed;
	
	// This one is the same each time.
	Random crater_randomizer(seed + 2);
	
	craters.resize(num_craters);
	for (CraterVector::iterator i = craters.begin(); i != craters.end(); ++ i)
	{
		Sphere3 & crater = * i;

		for (int timeout = 10; timeout > 0; -- timeout)
		{
			// Make a random crater.
			GenerateCreater(crater_randomizer, crater);
			
			// And for all the previously-created craters ...
			bool contained = false;
			for (CraterVector::const_iterator j = craters.begin(); j != i; ++ j)
			{
				// ... test whether one contains the other.
				if (Contains(crater, * j) || Contains(* j, crater))
				{
					// If so, it's wasteful to have around.
					contained = true;
					break;
				}
			}
			
			if (! contained)
			{
				break;
			}
			
			// Only try a fixed number of times to avoid waste.
			// We don't want to loop forever.
		}
	}
}

void sim::MoonShader::SetOrigin(Vector3d const & origin)
{
	center = moon.GetPosition() - origin;
}

void sim::MoonShader::InitRootPoints(form::Point * points[])
{
	int seed = moon.GetFormation().seed;
	
	// This one progresses with each iteration.
	Random point_randomizer(seed + 1);
	
	Scalar root_corner_length = root_three;
	for (int i = 0; i < 4; ++ i)
	{
		Vector3 position = root_corners[i] / root_corner_length;
		CalcPointPos(position);
		position += center;
		points[i]->pos = position;
	}
}

bool sim::MoonShader::InitMidPoint(form::Point & mid_point, form::Node const & a, form::Node const & b, int index) 
{
	Vector3 near_a = Vector3(a.GetCorner(TriMod(index + 1)).pos) - center;
	Vector3 near_b = Vector3(b.GetCorner(TriMod(index + 1)).pos) - center;	
	Vector3 near_mid = near_a + near_b;
	near_mid *= moon.GetRadiusMean() / Length(near_mid);
	
	Random crater_randomizer(moon.GetFormation().seed + 2);
	ApplyCraters(crater_randomizer, near_mid);
	
	near_mid += center;
	mid_point.pos = near_mid;
	
	return true;
}

// Comes in normalized. Is then given the correct length.
void sim::MoonShader::CalcPointPos(sim::Vector3 & position) const
{
	Scalar radius = moon.GetRadiusMean();
	position *= radius;
}

void sim::MoonShader::ApplyCraters(Random rnd, Vector3 & position) const
{
	Scalar t1, t2;
	Sphere3 crater;
	Ray3 ray(Vector3::Zero(), position);
	
	Scalar t = 1;
	
	for (CraterVector::const_iterator i = craters.begin(); i != craters.end(); ++ i)
	{
		Sphere3 const & crater = * i;
		
		Vector3 crater_to_pos = position - crater.center;
		Scalar crater_to_pos_distance_squared = LengthSq(crater_to_pos);
		
		if (crater_to_pos_distance_squared < Square(crater.radius))
		{
			if (! GetIntersection(crater, ray, t1, t2))
			{
				// If this happens only very rarely, it's probably just a precision thing.
				Assert(false);
				continue;
			}
			
			Assert (t2 >= 1);
			Assert (t1 <= 1);
			
			if (t1 < t)
			{
				t = t1;
			}
		}
	}
	
	position *= t;
}

void sim::MoonShader::GenerateCreater(Random & rnd, Sphere3 & crater) const
{
	Scalar moon_radius = moon.GetRadiusMean();
	//Scalar min_crater_distance = moon_radius * 1.05f;	// center to center
	Scalar max_crater_radius = moon_radius * .25;
	
	// First off, decide radius.
	Scalar crater_radius_coefficient = Power(rnd.GetUnitInclusive<float>(), 1.75f);
	crater.radius = max_crater_radius * crater_radius_coefficient;
	
	// Get a position which is within a radius=.5 sphere with even distribution.
	Scalar crater_center_squared;	// distance from moon center
	do
	{
		crater.center = Vector3(rnd.GetUnitInclusive<float>() - .5f, rnd.GetUnitInclusive<float>() - .5f, rnd.GetUnitInclusive<float>() - .5f);
		crater_center_squared = LengthSq(crater.center);
	}
	while (crater_center_squared > Square(.5f));
	
	// Push it to moon max radius and then out a [non]random amount up until crater.radius in distance.
	Scalar crater_elevation_coefficient = .85;
	Scalar desired_crater_center = moon_radius + crater_elevation_coefficient * crater.radius;
	crater.center *= desired_crater_center / Sqrt(crater_center_squared);
}


////////////////////////////////////////////////////////////////////////////////
// MoonShaderFactory

sim::MoonShaderFactory::MoonShaderFactory(Planet const & init_moon, int init_num_craters)
: moon(init_moon)
, num_craters(init_num_craters)
{
}

form::Shader * sim::MoonShaderFactory::Create(form::Formation const & formation) const
{
	return new MoonShader(moon, num_craters);
}

//
//  MoonShader.cpp
//  crag
//
//  Created by John on 8/9/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MoonShader.h"
#include "Planet.h"

#include "form/Formation.h"
#include "form/node/Node.h"
#include "form/node/Point.h"

#include "form/scene/Polyhedron.h"

#include "core/ConfigEntry.h"

#include "geom/Intersection.h"


namespace 
{
	
	sim::Scalar root_three = sqrt(3.);
	

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

sim::MoonShader::MoonShader(int seed, int num_craters, Scalar radius)
{
	// This one is the same each time.
	Random crater_randomizer(seed + 2);
	
	craters.resize(num_craters);
	for (CraterVector::iterator i = craters.begin(); i != craters.end(); ++ i)
	{
		Sphere3 & crater = * i;

		for (int timeout = 10; timeout > 0; -- timeout)
		{
			// Make a random crater.
			GenerateCreater(crater_randomizer, crater, radius);
			
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

void sim::MoonShader::InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const
{
	Scalar radius = polyhedron.GetShape().radius;
	
	Scalar root_corner_length = root_three;
	for (int i = 0; i < 4; ++ i)
	{
		Vector3 position = root_corners[i] / root_corner_length;
		position *= radius;
		position += polyhedron.GetShape().center;
		points[i]->pos = geom::Cast<float>(position);
	}
}

bool sim::MoonShader::InitMidPoint(form::Polyhedron & polyhedron, form::Node const & a, form::Node const & b, int index, form::Point & mid_point) const
{
	form::Formation const & formation = polyhedron.GetFormation();
	
	sim::Sphere3 const & shape = polyhedron.GetShape();
	Vector3 center = shape.center;
	Vector3 near_a = geom::Cast<Scalar>(a.GetCorner(TriMod(index + 1)).pos) - center;
	Vector3 near_b = geom::Cast<Scalar>(b.GetCorner(TriMod(index + 1)).pos) - center;
	Vector3 near_mid = near_a + near_b;
	near_mid *= shape.radius / Length(near_mid);
	
	Random crater_randomizer(formation.GetSeed() + 2);
	ApplyCraters(crater_randomizer, near_mid);
	
	near_mid += center;
	mid_point.pos = geom::Cast<float>(near_mid);
	
	return true;
}

void sim::MoonShader::ApplyCraters(Random rnd, Vector3 & position) const
{
	Scalar t1, t2;
	Ray3 ray(Vector3::Zero(), position);
	
	Scalar t = 1;
	
	for (CraterVector::const_iterator i = craters.begin(); i != craters.end(); ++ i)
	{
		Sphere3 const & crater = * i;
		
		Scalar crater_to_pos_distance_squared = DistanceSq(position, crater.center);
		
		if (crater_to_pos_distance_squared < Square(crater.radius))
		{
			if (! GetIntersection(crater, ray, t1, t2))
			{
				// If this happens only very rarely, it's probably just a precision thing.
				ASSERT(false);
				continue;
			}
			
			ASSERT (t2 >= 1);
			ASSERT (t1 <= 1);
			
			if (t1 < t)
			{
				t = t1;
			}
		}
	}
	
	position *= t;
}

void sim::MoonShader::GenerateCreater(Random & rnd, Sphere3 & crater, Scalar moon_radius) const
{
	//Scalar min_crater_distance = radius * 1.05f;	// center to center
	Scalar max_crater_radius = moon_radius * .25;
	
	// First off, decide radius.
	Scalar crater_radius_coefficient = pow(rnd.GetUnitInclusive<float>(), 1.75f);
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
	crater.center *= desired_crater_center / sqrt(crater_center_squared);
}

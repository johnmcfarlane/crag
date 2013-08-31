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

#include "form/Formation.h"
#include "form/Node.h"
#include "form/Point.h"
#include "form/Polyhedron.h"

#include "core/ConfigEntry.h"

#include "geom/Intersection.h"

using namespace sim;

namespace
{
	geom::abs::Scalar root_three = sqrt(3.);

	// RootNode initialization data, i.e. a tetrahedron.
	geom::abs::Vector3 root_corners[4] = 
	{
		geom::abs::Vector3(-1,  1,  1),
		geom::abs::Vector3( 1, -1,  1),
		geom::abs::Vector3( 1,  1, -1),
		geom::abs::Vector3(-1, -1, -1)
	};
}


////////////////////////////////////////////////////////////////////////////////
// MoonShader

MoonShader::MoonShader(int seed, int num_craters, geom::abs::Scalar radius)
{
	// This one is the same each time.
	Random crater_randomizer(seed + 2);
	
	craters.resize(num_craters);
	for (CraterVector::iterator i = craters.begin(); i != craters.end(); ++ i)
	{
		geom::abs::Sphere3 & crater = * i;

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

void MoonShader::InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const
{
	geom::abs::Scalar radius = polyhedron.GetShape().radius;
	
	geom::abs::Scalar root_corner_length = root_three;
	for (int i = 0; i < 4; ++ i)
	{
		geom::abs::Vector3 position = root_corners[i] / root_corner_length;
		position *= radius;
		position += polyhedron.GetShape().center;
		points[i]->pos = geom::Cast<float>(position);
	}
}

bool MoonShader::InitMidPoint(form::Polyhedron & polyhedron, form::Node const & a, form::Node const & b, int index, form::Point & mid_point) const
{
	form::Formation const & formation = polyhedron.GetFormation();
	
	geom::abs::Sphere3 const & shape = polyhedron.GetShape();
	geom::abs::Vector3 center = shape.center;
	geom::abs::Vector3 near_a = geom::Cast<geom::abs::Scalar>(a.GetCorner(TriMod(index + 1)).pos) - center;
	geom::abs::Vector3 near_b = geom::Cast<geom::abs::Scalar>(b.GetCorner(TriMod(index + 1)).pos) - center;
	geom::abs::Vector3 near_mid = near_a + near_b;
	near_mid *= shape.radius / Length(near_mid);
	
	Random crater_randomizer(formation.GetSeed() + 2);
	ApplyCraters(crater_randomizer, near_mid);
	
	near_mid += center;
	mid_point.pos = geom::Cast<float>(near_mid);
	
	return true;
}

void MoonShader::ApplyCraters(Random, geom::abs::Vector3 & position) const
{
	geom::abs::Scalar t1, t2;
	geom::abs::Ray3 ray(geom::abs::Vector3::Zero(), position);
	
	geom::abs::Scalar t = 1;
	
	for (CraterVector::const_iterator i = craters.begin(); i != craters.end(); ++ i)
	{
		geom::abs::Sphere3 const & crater = * i;
		
		geom::abs::Scalar crater_to_pos_distance_squared = DistanceSq(position, crater.center);
		
		if (crater_to_pos_distance_squared < Squared(crater.radius))
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

void MoonShader::GenerateCreater(Random & rnd, geom::abs::Sphere3 & crater, geom::abs::Scalar moon_radius) const
{
	//geom::abs::Scalar min_crater_distance = radius * 1.05f;	// center to center
	geom::abs::Scalar max_crater_radius = moon_radius * .25;
	
	// First off, decide radius.
	geom::abs::Scalar crater_radius_coefficient = pow(rnd.GetUnitInclusive<float>(), 1.75f);
	crater.radius = max_crater_radius * crater_radius_coefficient;
	
	// Get a position which is within a radius=.5 sphere with even distribution.
	geom::abs::Scalar crater_center_squared;	// distance from moon center
	do
	{
		crater.center = geom::abs::Vector3(rnd.GetUnitInclusive<float>() - .5f, rnd.GetUnitInclusive<float>() - .5f, rnd.GetUnitInclusive<float>() - .5f);
		crater_center_squared = LengthSq(crater.center);
	}
	while (crater_center_squared > Squared(.5f));
	
	// Push it to moon max radius and then out a [non]random amount up until crater.radius in distance.
	geom::abs::Scalar crater_elevation_coefficient = .85;
	geom::abs::Scalar desired_crater_center = moon_radius + crater_elevation_coefficient * crater.radius;
	crater.center *= desired_crater_center / std::sqrt(crater_center_squared);
}

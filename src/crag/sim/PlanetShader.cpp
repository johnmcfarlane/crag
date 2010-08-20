/*
 *  PlanetShader.cpp
 *  Crag
 *
 *  Created by John on 2/21/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Planet.h"
#include "PlanetShader.h"

#include "form/Formation.h"
#include "form/node/Node.h"
#include "form/node/Point.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

#include "geom/VectorOps.h"
#include "geom/SphereOps.h"

#include "gfx/Color.h"
#include "gfx/Debug.h"


namespace 
{
	
	// Config values
	CONFIG_DEFINE (planet_shader_depth_medium, int, 3);
	CONFIG_DEFINE (planet_shader_depth_deep, int, 6);
	CONFIG_DEFINE (planet_shader_error_co, double, 0.995);
	//CONFIG_DEFINE (formation_color, gfx::Color4f, gfx::Color4f(1.f, 1.f, 1.f));
	
	sim::Scalar root_three = Sqrt(3.);
	
	// RootNode initialization data
	sim::Vector3 root_corners[4] = 
	{
		sim::Vector3(-1, -1, -1),
		sim::Vector3(-1,  1,  1),
		sim::Vector3( 1, -1,  1),
		sim::Vector3( 1,  1, -1)
	};
	
	// InitMidPoint helper
	int MeasureDepth(form::Node const * n, int max_depth)
	{
		int depth = 0;
		
		while (true)
		{
			n = n->parent;			
			if (n == nullptr)
			{
				break;
			}
			
			++ depth;
			if (depth >= max_depth)
			{
				break;
			}
		}
		
		return depth;
	}

}


////////////////////////////////////////////////////////////////////////////////
// PlanetShader

sim::PlanetShader::PlanetShader(Planet const & init_planet)
: center(Vector3::Zero())
, planet(init_planet)
{
}

void sim::PlanetShader::SetOrigin(Vector3d const & origin)
{
	center = planet.GetPosition() - origin;
}

void sim::PlanetShader::InitRootPoints(form::Point * points[])
{
	int seed = planet.GetFormation().seed;
	
	// This one progresses with each iteration.
	Random point_randomizer(seed + 1);
	
	// This one is the same each time.
	Random crater_randomizer(seed + 2);
	
	Scalar root_corner_length = root_three;
	for (int i = 0; i < 4; ++ i)
	{
		Vector3 position = root_corners[i] / root_corner_length;
		CalcRootPointPointPos(point_randomizer, position);
		position += center;
		points[i]->pos = position;
	}
}

void sim::PlanetShader::InitMidPoint(int i, form::Node const & a, form::Node const & b, form::Point & mid_point) 
{
	int depth = MeasureDepth(& a, planet_shader_depth_deep);
	Assert(depth == MeasureDepth(& b, planet_shader_depth_deep));
	
	Params params;
	params.near_a = Vector3(a.GetCorner(TriMod(i + 1)).pos) - center;
	params.near_b = Vector3(b.GetCorner(TriMod(i + 1)).pos) - center;
	params.far_a = Vector3(a.GetCorner(i).pos) - center;
	params.far_b = Vector3(b.GetCorner(i).pos) - center;
	
	//params.near_mid = params.near_a + params.near_b;
	//params.near_mid *= planet.GetRadiusAverage() / Length(params.near_mid);
	
	params.depth = static_cast<Scalar> (depth) / planet_shader_depth_deep;
	Assert(params.depth >= 0 && params.depth <= 1);
	
	int seed_1 = Random(a.seed + i).GetInt();
	int seed_2 = Random(b.seed + i).GetInt();
	int combined_seed(seed_1 + seed_2);
	Random rnd(combined_seed);
	params.rnd = Random(combined_seed);
	
	Vector3 result;
	if (depth >= planet_shader_depth_medium)
	{
		if (depth >= planet_shader_depth_deep)
		{
			result = CalcMidPointPos_Deep(params);
			mid_point.col = gfx::Color4b::Red();
		}
		else
		{
			result = CalcMidPointPos_Medium(params);
			mid_point.col = gfx::Color4b::Blue();
		}
	}
	else 
	{
		result = CalcMidPointPos_Shallow(params);
		mid_point.col = gfx::Color4b::Green();
	}
	
/*#if ! defined(NDEBUG)
	// Check that resultant point is within the [min, max] range.
	Scalar altitude1 = Length(mid_point.pos - sim::Vector3(center));
	Scalar altitude2 = Length(Vector3(mid_point.pos) - center);
	if (Min(altitude1, altitude2) < planet.GetRadiusMin())
	{
		Assert(false);
	}
	if (Max(altitude1, altitude2) > planet.GetRadiusMax())
	{
		Assert(false);
	}
#endif*/
	
	result += center;
	mid_point.pos = result;
	mid_point.col = gfx::Color4b::White();
}

// Comes in normalized. Is then given the correct length.
void sim::PlanetShader::CalcRootPointPointPos(Random & rnd, sim::Vector3 & position) const
{
	Scalar radius = planet.GetRadiusAverage();
	position *= radius;
}

// At shallow depth, heigh is highly random.
sim::Vector3 sim::PlanetShader::CalcMidPointPos_Shallow(Params & params) const
{
	Scalar radius_min = planet.GetRadiusMin();
	Scalar radius_max = planet.GetRadiusMax();
	//Scalar radius = planet.GetRadiusAverage();
	Scalar radius_range = radius_max - radius_min;
	
	// Do the random stuff to get the radius.
	Scalar rnd_x = params.rnd.GetFloatInclusive() * 2. - 1.;	// Get a random number in the range [-1, 1]
	rnd_x *= Square(rnd_x);				// Bias the random number towards 0.
	rnd_x *= planet_shader_error_co;	// Make sure a precision error pushes us beyond the [min - max] range.
	Scalar rnd_n = (rnd_x * .5) + .5;	// Shift into the range: [0, 1].

	Scalar radius = radius_min + radius_range * rnd_n;	// Shift into the range [radius_min, radius_max].
	Assert(radius >= radius_min);
	Assert(radius <= radius_max);
	
	Vector3 a = params.near_a;
	Vector3 b = params.near_b;
	Vector3 v = (a + b) * .5;
	Scalar length = Length(v);
	v *= (radius / length);
	
	return v;
}

sim::Vector3 sim::PlanetShader::CalcMidPointPos_Medium(Params & params) const 
{
	Scalar near_a_altitude = Length(params.near_a);
	Scalar near_b_altitude = Length(params.near_b);
	Scalar near_distance = Length(params.near_a - params.near_b);

	Scalar far_a_altitude = Length(params.far_a);
	Scalar far_b_altitude = Length(params.far_b);
	
//	Scalar far_weight = 0;//near_distance / far_distance;
//	Scalar far_weight_2 = far_weight * 2.;
//	Scalar altitude = (near_a_altitude + near_b_altitude + far_weight * (far_a_altitude + far_b_altitude)) * (1. / (2. + far_weight_2));
	Scalar altitude = (near_a_altitude + near_b_altitude + far_a_altitude + far_b_altitude) * .25;
	Vector3 directional = params.near_a + params.near_b;
	Scalar directional_length = Length(directional);
		
	Scalar rnd_x = params.rnd.GetFloatInclusive() * 2. - 1.;
	rnd_x *= Square(rnd_x);
	
	// Figure out how much the altitude may be varied in either direction,
	// and clip that variance based on the hard limits of the planet.
	// Actually, clip it to half of that to make it look less like a hard limit.
	// And do the clipping based on how far the variance /might/ go.
	Scalar altitude_variance_coefficient = near_distance * .05;
	if (rnd_x > 0)
	{
		Scalar max_altitude = altitude + altitude_variance_coefficient;
		Scalar max_allowed_altitude = planet.GetRadiusMax() - 1;
		if (max_altitude > max_allowed_altitude)
		{
			altitude_variance_coefficient = (max_allowed_altitude - altitude) * .5;
		}
	}
	else if (rnd_x < 0)
	{
		Scalar min_altitude = altitude - altitude_variance_coefficient;
		Scalar min_allowed_altitude = planet.GetRadiusMin() + 1;
		if (min_altitude < min_allowed_altitude)
		{
			altitude_variance_coefficient = (altitude - min_allowed_altitude) * .5;
		}
	}
	
	altitude += rnd_x * altitude_variance_coefficient;
	
	directional *= (altitude / directional_length);
	return directional;
}

// TODO: Maybe avoid doing the -/+ center by crossing near and far?
sim::Vector3 sim::PlanetShader::CalcMidPointPos_Deep(Params & params) const 
{
	Vector3 inter_near = params.near_a - params.near_b;
	Vector3 inter_far = params.far_a - params.far_b;
	Vector3 outward = CrossProduct(inter_far, inter_near);
	Scalar outward_length = Length(outward);
	
	Scalar random = (static_cast<Scalar>(params.rnd.GetFloatInclusive()) - .5) * Length(inter_near) * .05;
	
	sim::Vector3 avg = (params.near_a + params.near_b) * .5;
	avg += outward * (random / outward_length);
	return avg;
}


////////////////////////////////////////////////////////////////////////////////
// PlanetShaderFactory

sim::PlanetShaderFactory::PlanetShaderFactory(Planet const & init_planet)
: planet(init_planet)
{
}

form::Shader * sim::PlanetShaderFactory::Create(form::Formation const & formation) const
{
	return new PlanetShader(planet);
}

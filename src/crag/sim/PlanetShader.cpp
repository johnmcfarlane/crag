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

//#include "form/Formation.h"
#include "form/node/Node.h"
#include "form/node/Point.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"
#include "geom/VectorOps.h"

#include "gfx/Color.h"
#include "gfx/Debug.h"


namespace 
{
	
	// Config values
	CONFIG_DEFINE (planet_shader_depth_medium, int, 2);
	CONFIG_DEFINE (planet_shader_depth_deep, int, 6);
	CONFIG_DEFINE (planet_shader_error_co, double, 0.995);
	//CONFIG_DEFINE (formation_color, gfx::Color4f, gfx::Color4f(1.f, 1.f, 1.f));
	
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

sim::PlanetShader::PlanetShader(Planet const & init_planet, int init_num_craters)
: center(Vector3::Zero())
, planet(init_planet)
, num_craters(init_num_craters)
{
}

void sim::PlanetShader::SetOrigin(Vector3d const & origin)
{
	center = planet.GetPosition() - origin;
}

void sim::PlanetShader::InitRootPoints(int seed, form::Point * points[])
{
	Scalar radius = planet.GetAverageRadius();
	Scalar root_corner_length = Sqrt(3.);
	Scalar position_coefficient = radius / root_corner_length;
	
	for (int i = 0; i < 4; ++ i)
	{
		points[i]->pos = root_corners[i] * position_coefficient + center;
	}
}

void sim::PlanetShader::InitMidPoint(int i, form::Node const & a, form::Node const & b, form::Point & mid_point) 
{
	int depth = MeasureDepth(& a, planet_shader_depth_deep);
	Assert(depth == MeasureDepth(& b, planet_shader_depth_deep));
	
	int seed_1 = Random(a.seed + i).GetInt();
	int seed_2 = Random(b.seed + i).GetInt();
	int combined_seed(seed_1 + seed_2);
	Random rnd(combined_seed);
	
	Params params = 
	{
		rnd,
		Vector3(a.GetCorner(TriMod(i + 1)).pos) - center,
		Vector3(b.GetCorner(TriMod(i + 1)).pos) - center,
		Vector3(a.GetCorner(i).pos) - center,
		Vector3(b.GetCorner(i).pos) - center
	};
	
	Vector3 result;
	if (depth >= planet_shader_depth_medium)
	{
		if (false && depth >= planet_shader_depth_deep)
		{
			result = CalcMidPointPos_Deep(params);
		}
		else
		{
			result = CalcMidPointPos_Medium(params);
		}
	}
	else 
	{
		result = CalcMidPointPos_Shallow(params);
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
	
	Scalar planet_radius = planet.GetRadiusMax();
	Sphere3 crater;
	Random poo(77);
	for (int i = 25; i; -- i)
	{
		crater.center = Vector3(poo.GetFloatInclusive() - .5f, poo.GetFloatInclusive() - .5f, poo.GetFloatInclusive() - .5f);
		crater.center *= planet_radius * 1.1 / Length(crater.center);
		crater.radius = poo.GetFloatInclusive() * planet_radius * .1;
		
		Vector3 center_to_result = result - crater.center;
		Scalar length = Length(center_to_result);
		if (length < crater.radius)
		{
			center_to_result *= crater.radius / length;
			result = center_to_result + crater.center;
		}
	}

	result += center;
	mid_point.pos = result;
}

sim::Vector3 sim::PlanetShader::CalcRootPointPointPos(Params & params)
{
}

// At shallow depth, heigh is highly random.
sim::Vector3 sim::PlanetShader::CalcMidPointPos_Shallow(Params & params) 
{
	Scalar radius_min = planet.GetRadiusMin();
	Scalar radius_max = planet.GetRadiusMax();
	//Scalar radius = planet.GetAverageRadius();
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

sim::Vector3 sim::PlanetShader::CalcMidPointPos_Medium(Params & params) 
{
	Scalar near_a_altitude = Length(params.near_a);
	Scalar near_b_altitude = Length(params.near_b);
	Scalar near_distance = Length(params.near_a - params.near_b);

	Scalar far_a_altitude = Length(params.far_a);
	Scalar far_b_altitude = Length(params.far_b);
	
	Scalar far_weight = 0;//near_distance / far_distance;
	Scalar far_weight_2 = far_weight * 2.;
	Scalar altitude = (near_a_altitude + near_b_altitude + far_weight * (far_a_altitude + far_b_altitude)) * (1. / (2. + far_weight_2));
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
sim::Vector3 sim::PlanetShader::CalcMidPointPos_Deep(Params & params) 
{
	sim::Vector3 avg = (params.far_a + params.far_b) * .5;
	return avg;

#if 0
	Random rnd(seed);
	
	/*Scalar near_weight = 1. ;/// .5;
	Scalar far_weight = 1. ;/// Sqrt(1 - Square(.5));
	Scalar sum_weight = near_weight + far_weight;*/
	
	Vector3 near_corner1_d = near_corner1;
	Vector3 near_corner2_d = near_corner2;
	Vector3 far_corner1_d = far_corner1;
	Vector3 far_corner2_d = far_corner2;

	near_corner1_d -= center;
	near_corner2_d -= center;
	far_corner1_d -= center;
	far_corner2_d -= center;

	/*Vector3 avg_corner = ((near_corner1_d + near_corner2_d) * near_weight + (far_corner1_d + far_corner2_d) * far_weight)
	/ (2. * sum_weight);*/
	Vector3 avg_corner = Normalized(far_corner1_d + far_corner2_d) * (Scalar)formation.scale;
	
	Vector3 unit = Normalized(avg_corner - center);
	
	/*	Scalar near_height1 = Length(near_corner1_d - center);
	 Scalar near_height2 = Length(near_corner2_d - center);
	 Scalar far_height1 = Length(far_corner1_d - center);
	 Scalar far_height2 = Length(far_corner2_d - center);*/
	//	Scalar min_height = Min(Min(height1, height2), Min(height3, height4));
	//	Scalar max_height = Max(Max(height1, height2), Max(height3, height4));
	//	Scalar avg_height = (min_height + max_height) * static_cast<Scalar>(.5);
	//	Scalar avg_height = (min_height + max_height) * static_cast<Scalar>(.5);
	Scalar avg_height = Length(unit);
	
	Scalar variance = 0;//LengthSq(near_corner1 - near_corner2) * static_cast<Scalar>(.000001);
	
	Scalar height = avg_height + variance * (rnd.GetFloatInclusive() - rnd.GetFloatInclusive());
	if (height > max_radius)
	{
		max_radius = height;
	}
	
	Vector3 result = center + unit * height;
	return avg_corner + center;//result;
#endif
}


////////////////////////////////////////////////////////////////////////////////
// PlanetShader

sim::PlanetShaderFactory::PlanetShaderFactory(Planet const & init_planet, int init_num_craters)
: planet(init_planet)
, num_craters(init_num_craters)
{
}

form::Shader * sim::PlanetShaderFactory::Create(form::Formation const & formation) const
{
	return new PlanetShader(planet, num_craters);
}

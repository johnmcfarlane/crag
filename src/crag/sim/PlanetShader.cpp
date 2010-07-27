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
	CONFIG_DEFINE (planet_shader_depth_medium, int, 6);
	CONFIG_DEFINE (planet_shader_depth_deep, int, 10);
	CONFIG_DEFINE (formation_color, gfx::Color4f, gfx::Color4f(1.f, 1.f, 1.f));
	
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
	int seed_1 = Random(a.seed + i).GetInt();
	int seed_2 = Random(b.seed + i).GetInt();
	int combined_seed(seed_1 + seed_2);
	
	int depth = MeasureDepth(& a, planet_shader_depth_deep);
	Assert(depth == MeasureDepth(& b, planet_shader_depth_deep));
	
	Vector3f const & near_a = a.GetCorner(TriMod(i + 1)).pos;
	Vector3f const & near_b = b.GetCorner(TriMod(i + 1)).pos;

	if (depth >= planet_shader_depth_medium)
	{
		Vector3f const & far_a = a.GetCorner(i).pos;
		Vector3f const & far_b = b.GetCorner(i).pos;
		
		if (false && depth >= planet_shader_depth_deep)
		{
			mid_point.pos = CalcMidPointPos_Deep(combined_seed, near_a, near_b, far_a, far_b);
		}
		else
		{
			mid_point.pos = CalcMidPointPos_Medium(combined_seed, near_a, near_b, far_a, far_b);
		}
	}
	else 
	{
		mid_point.pos = CalcMidPointPos_Shallow(combined_seed, near_a, near_b);
	}
}

// At shallow depth, heigh is highly random.
Vector3f sim::PlanetShader::CalcMidPointPos_Shallow(int seed, 
													Vector3f const & near_a, 
													Vector3f const & near_b) 
{
	Random rnd(seed);

	Scalar radius_min = planet.GetRadiusMin();
	Scalar radius_max = planet.GetRadiusMax ();
	//Scalar radius = planet.GetAverageRadius();
	Scalar radius_range = radius_max - radius_min;
	
	// Do the random stuff to get the radius.
#if 0
	Scalar rnd_x = rnd.GetFloatInclusive() * 2. - 1.;
	Scalar rnd_a = acos(rnd_x);	// The two nice things about acos is that it's center-biased and it doesn't tend to infinity like most bell-shaped curves.
	Scalar rnd_n = rnd_a / PI;
#else
	Scalar rnd_x = rnd.GetFloatInclusive() * 2. - 1.;
	rnd_x *= Square(rnd_x);
	Scalar rnd_n = (rnd_x * .5) + .5;
#endif
	Scalar radius = radius_min + radius_range * rnd_n;
	Assert(radius >= radius_min);
	Assert(radius <= radius_max);
	
	Vec3 a = near_a;
	Vec3 b = near_b;
	Vec3 v = (a + b) * .5;
	v -= center;
	S length = Length(v);
	v *= radius / length;
	v += center;
	
	return v;
}

Vector3f sim::PlanetShader::CalcMidPointPos_Medium(int seed, 
												 Vector3f const & near_a, Vector3f const & near_b, 
												 Vector3f const & far_a, Vector3f const & far_b) 
{
	Vec3 near_a_relative = near_a;
	S near_a_altitude = Length(near_a_relative -= center);
	Vec3 near_b_relative = near_b;
	S near_b_altitude = Length(near_b_relative -= center);
	S near_distance = Length(near_a - near_b);

	Vec3 far_a_relative = far_a;
	S far_a_altitude = Length(far_a_relative -= center);
	Vec3 far_b_relative = far_b;
	S far_b_altitude = Length(far_b_relative -= center);
	S far_distance = Length(far_a - far_b);
	
	S far_weight = 0;//near_distance / far_distance;
	S far_weight_2 = far_weight * 2.;
	S altitude = (near_a_altitude + near_b_altitude + far_weight * (far_a_altitude + far_b_altitude)) * (1. / (2. + far_weight_2));
	Vec3 directional = near_a_relative + near_b_relative;
	S directional_length = Length(directional);
	
	Random rnd(seed);	
	Scalar rnd_x = rnd.GetFloatInclusive() * 2. - 1.;
	rnd_x *= Square(rnd_x);
	altitude += rnd_x * near_distance * .05;
	
	directional *= (altitude / directional_length);
	directional += center;
	return directional;
}

// TODO: Maybe avoid doing the -/+ center by crossing near and far?
Vector3f sim::PlanetShader::CalcMidPointPos_Deep(int seed, 
												 Vector3f const & near_corner1, Vector3f const & near_corner2, 
												 Vector3f const & far_corner1, Vector3f const & far_corner2) 
{
	Vector3f avg = (far_corner1 + far_corner2) * .5f;
	return avg;

#if 0
	Random rnd(seed);
	
	/*S near_weight = 1. ;/// .5;
	S far_weight = 1. ;/// Sqrt(1 - Square(.5));
	S sum_weight = near_weight + far_weight;*/
	
	Vec3 near_corner1_d = near_corner1;
	Vec3 near_corner2_d = near_corner2;
	Vec3 far_corner1_d = far_corner1;
	Vec3 far_corner2_d = far_corner2;

	near_corner1_d -= center;
	near_corner2_d -= center;
	far_corner1_d -= center;
	far_corner2_d -= center;

	/*Vec3 avg_corner = ((near_corner1_d + near_corner2_d) * near_weight + (far_corner1_d + far_corner2_d) * far_weight)
	/ (2. * sum_weight);*/
	Vec3 avg_corner = Normalized(far_corner1_d + far_corner2_d) * (S)formation.scale;
	
	Vec3 unit = Normalized(avg_corner - center);
	
	/*	S near_height1 = Length(near_corner1_d - center);
	 S near_height2 = Length(near_corner2_d - center);
	 S far_height1 = Length(far_corner1_d - center);
	 S far_height2 = Length(far_corner2_d - center);*/
	//	S min_height = Min(Min(height1, height2), Min(height3, height4));
	//	S max_height = Max(Max(height1, height2), Max(height3, height4));
	//	S avg_height = (min_height + max_height) * static_cast<S>(.5);
	//	S avg_height = (min_height + max_height) * static_cast<S>(.5);
	S avg_height = Length(unit);
	
	S variance = 0;//LengthSq(near_corner1 - near_corner2) * static_cast<S>(.000001);
	
	S height = avg_height + variance * (rnd.GetFloatInclusive() - rnd.GetFloatInclusive());
	if (height > max_radius)
	{
		max_radius = height;
	}
	
	Vec3 result = center + unit * height;
	return avg_corner + center;//result;
#endif
}

#if defined(FATHOM_IT_OUT)
void majik_fn()
{
	if (done)
	{
		gfx::Debug::ColorPair cp(gfx::Color4f(1,0,0), gfx::Color4f(0,1,0));
		gfx::Debug::AddLine(q, e, cp);
		gfx::Debug::AddLine(w, e, cp);
		gfx::Debug::AddLine(q, r, cp);
		gfx::Debug::AddLine(w, r, cp);
	}
}
#endif


////////////////////////////////////////////////////////////////////////////////
// PlanetShader

sim::PlanetShaderFactory::PlanetShaderFactory(Planet const & init_planet)
: planet(init_planet)
{
}

form::Shader * sim::PlanetShaderFactory::Create(form::Formation const & formation) const
{
	return new PlanetShader(planet);
}

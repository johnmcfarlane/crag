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

#include "PlanetShader.h"

#include "form/Formation.h"
#include "form/Node.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"
#include "core/VectorOps.h"

#include "gfx/Color.h"


namespace ANONYMOUS {

CONFIG_DEFINE (formation_color, gfx::Color4f, gfx::Color4f(1.f, 1.f, 1.f));

}


////////////////////////////////////////////////////////////////////////////////
// PlanetShader

sim::PlanetShader::PlanetShader(form::Formation const & init_formation)
: center(Vector3::Zero())
, max_radius(0)
, formation(init_formation)
{
}

void sim::PlanetShader::SetOrigin(Vector3d const & origin)
{
	center = formation.position - origin;
}

void sim::PlanetShader::InitMidPoint(int i, form::Node const & a, form::Node const & b, Vector3f & mid_point) 
{
	int seed_1 = Random(a.seed + i).GetInt();
	int seed_2 = Random(b.seed + i).GetInt();
	int combined_seed(seed_1 + seed_2);
	
	Vector3f const & near_a = a.GetCorner(TriMod(i + 1));
	Vector3f const & near_b = b.GetCorner(TriMod(i + 1));
	Vector3f const & far_a = a.GetCorner(i);
	Vector3f const & far_b = b.GetCorner(i);
	mid_point = CalcMidPointPos(combined_seed, near_a, near_b, far_a, far_b);
	
#if 0
#if defined(ALTITUDE_COLORS)
	float altitude_square = LengthSq(mid_point.pos) / Square(scale);
	if (altitude_square < 1.005f)
	{
		mid_point.red = 224;
		mid_point.green = 224;
		mid_point.blue = 48;
	}
	else
	{
		if (altitude_square > 1.03f)
		{
			mid_point.red = 255;
			mid_point.green = 255;
			mid_point.blue = 255;
		}
		else
		{
			mid_point.red = 96;
			mid_point.green = 192;
			mid_point.blue = 16;
		}
	}
#else
	mid_point.red = static_cast<form::Point::ColorComp>(formation_color.r * 255);
	mid_point.green = static_cast<form::Point::ColorComp>(formation_color.g * 255);
	mid_point.blue = static_cast<form::Point::ColorComp>(formation_color.b * 255);
#endif
#endif
	
#if defined(HACKY_OCCLUSION)
	// The direction of the surface (away or towards the formation center).
	Vector3f p_a = Normalized(near_a - mid_point.pos);
	Vector3f p_b = Normalized(near_b - mid_point.pos);
	Vector3f point_direction = (p_a + p_b) * .5f;
	float pointiness = DotProduct(point_direction, Normalized(mid_point.pos));
	float anti_occlusion = (1.f - pointiness) * .5f;
	
	// Hacky occlusion test
	float const min_occlusion_factor = 0.0f;
	float const max_occlusion_factor = 2.0f;
	float occlusion_factor = Min(1.f, min_occlusion_factor + anti_occlusion * (max_occlusion_factor - min_occlusion_factor));
	Assert(occlusion_factor >= 0 && occlusion_factor <= 1);
	
	mid_point.red *= occlusion_factor;
	mid_point.green *= occlusion_factor;
	mid_point.blue *= occlusion_factor;
#endif
	
#if defined(DEBUG_INDEX)
	mid_point.red = mid_point.green = mid_point.blue = 0;
	int on = 128;
	switch (i)
	{
		case 0:
			mid_point.red = on;
			break;
		case 1:
			mid_point.green = on;
			break;
		case 2:
			mid_point.blue = on;
			break;
	}
#endif
	
//	mid_point.flags = 0;
}

Vector3f sim::PlanetShader::CalcMidPointPos(int seed, Vector3f const & near_corners1, Vector3f const & near_corners2, Vector3f const & /*far_corners1*/, Vector3f const & /*far_corners2*/) 
{
	Random rnd(seed);
	
	Vec3 near_corners1_d = near_corners1;
	Vec3 near_corners2_d = near_corners2;
	
	Vec3 unit = Normalized(near_corners1_d + near_corners2_d - (center * static_cast<S>(2)));
	
	S height1 = Length(near_corners1_d - center);
	S height2 = Length(near_corners2_d - center);
	S min_height = Min(height1, height2);
	S max_height = Max(height1, height2);
	S avg_height = (min_height + max_height) * static_cast<S>(.5);
	
	S variance = Length(near_corners1 - near_corners2) * static_cast<S>(.015);
	
	S height = avg_height + variance * (rnd.GetFloatInclusive() - rnd.GetFloatInclusive());
	if (height > max_radius)
	{
		max_radius = height;
	}
	
	Vec3 result = center + unit * height;
	return result;
}


////////////////////////////////////////////////////////////////////////////////
// PlanetShader

form::Shader * sim::PlanetShaderFactory::Create(form::Formation const & formation) const
{
	return new PlanetShader(formation);
}

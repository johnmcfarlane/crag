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
#include "geom/VectorOps.h"

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
}

Vector3f sim::PlanetShader::CalcMidPointPos(int seed, Vector3f const & near_corners1, Vector3f const & near_corners2, Vector3f const & far_corners1, Vector3f const & far_corners2) 
{
	Random rnd(seed);
	
	Vec3 near_corners1_d = near_corners1;
	Vec3 near_corners2_d = near_corners2;
	Vec3 far_corners1_d = far_corners1;
	Vec3 far_corners2_d = far_corners2;
	
	Vec3 unit = Normalized(near_corners1_d + near_corners2_d - (center * static_cast<S>(2)));
	
	S height1 = Length(near_corners1_d - center);
	S height2 = Length(near_corners2_d - center);
	S height3 = Length(far_corners1_d - center);
	S height4 = Length(far_corners2_d - center);
	S min_height = Min(Min(height1, height2), Min(height3, height4));
	S max_height = Max(Max(height1, height2), Max(height3, height4));
	S avg_height = (min_height + max_height) * static_cast<S>(.5);
	
	S variance = Length(far_corners1 - far_corners2) * static_cast<S>(.0065);
	
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

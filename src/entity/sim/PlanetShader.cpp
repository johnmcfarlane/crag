//
//  PlanetShader.cpp
//  crag
//
//  Created by John on 2/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "PlanetShader.h"

#include "sim/Engine.h"

#include "form/Formation.h"
#include "form/Node.h"
#include "form/Point.h"
#include "form/Polyhedron.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

#include "gfx/Color.h"
#include "gfx/Debug.h"

using namespace sim;

namespace 
{
	// Config values
	CONFIG_DEFINE(planet_shader_depth_medium, 2);
	CONFIG_DEFINE(planet_shader_random_range, 0.002);
	CONFIG_DEFINE(planet_shader_medium_coefficient, .35);
	
	geom::uni::Scalar root_three = sqrt(3.);
	
	// RootNode initialization data. A tetrahedron.
	geom::uni::Vector3 root_corners[4] =
	{
		geom::uni::Vector3(-1,  1,  1),
		geom::uni::Vector3( 1, -1,  1),
		geom::uni::Vector3( 1,  1, -1),
		geom::uni::Vector3(-1, -1, -1)
	};
	
	// InitMidPoint helper
	int MeasureDepth(form::Node const * n, int max_depth)
	{
		int depth = 0;
		
		for (;;)
		{
			n = n->GetParent();
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


namespace debug
{
#if defined(CRAG_RELEASE) || 1
	void ClearNodePoints() { }
	void MarkNodePoint(form::Node const &, form::Point const &, int /*row*/, int /*column*/) { }
	void DrawNodePoints() { }
	void ClampNodePoints() { }
#else
	struct Line
	{
		Line(form::Vector3 const & init_a, form::Vector3 const & init_b, int row, int column) 
		: a(init_a)
		, b(init_b) 
		, row(row)
		, column(column)
		{ 
		}
		
		form::Vector3 a;
		form::Vector3 b;
		int row;
		int column;
	};
	
	typedef std::vector<Line> LineVector;
	LineVector lines;
	std::mutex m;
	int count_down = 50000;
	
	void ClearNodePoints() 
	{
		if (! count_down)
		{
			return;
		}
		m.Lock();
		lines.clear();
		m.Unlock();
	}
	
	void MarkNodePoint(form::Node const & node, form::Point const & point, int row, int column) 
	{
		if (! count_down)
		{
			return;
		}
		m.Lock();
		lines.push_back(Line(node.center, point.pos, row, column));
		m.Unlock();
	}
	
	void DrawNodePoints() 
	{ 
		m.Lock();
		for (LineVector::const_iterator i = lines.begin(); i != lines.end(); ++ i)
		{
			Line const & line = * i;
			gfx::Color4f c(static_cast<float> (line.row) * .25f, static_cast<float> (line.column) * .25f, 1);
			gfx::Debug::AddLine(line.a, line.b, gfx::Debug::ColorPair(c, c), gfx::Debug::ColorPair(gfx::Color4f::White(), gfx::Color4f::White()));
		}
		m.Unlock();
	}
	
	void ClampNodePoints()
	{
		if (count_down)
		{
			-- count_down;
		}
	}
#endif
}


////////////////////////////////////////////////////////////////////////////////
// PlanetShader::Params

struct PlanetShader::Params
{
	Params(Params const &) noexcept = delete;
	Params & operator=(Params const &) noexcept = delete;

	form::Node const & a;
	form::Node const & b;
	int const index;
	int const depth;	// as a proportion of planet_shader_depth_deep
	Random rnd;
};


////////////////////////////////////////////////////////////////////////////////
// PlanetShader

void PlanetShader::InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const
{
	int seed = polyhedron.GetFormation().GetSeed();
	
	// This one progresses with each iteration.
	Random point_randomizer(seed + 1);
	
	// This one is the same each time.
	//Random crater_randomizer(seed + 2);

	auto shape = static_cast<geom::uni::Sphere3>(polyhedron.GetShape());
	geom::uni::Scalar inverse_root_corner_length = 1. / root_three;
	for (int i = 0; i < 4; ++ i)
	{
		geom::uni::Vector3 position = root_corners[i] * inverse_root_corner_length;
		CalcRootPointPos(point_randomizer, position);
		position *= shape.radius;
		position += shape.center;
		points[i]->pos = static_cast<form::Point::Vector3>(position);
	}
}

bool PlanetShader::InitMidPoint(form::Polyhedron & polyhedron, form::Node const & a, form::Node const & b, int index, form::Point & mid_point) const
{
	CRAG_VERIFY(a);
	CRAG_VERIFY(b);

	int max_depth = std::numeric_limits<int>::max();
	int depth = MeasureDepth(& a, max_depth);
	ASSERT(depth == MeasureDepth(& b, max_depth));

	int seed_1 = Random(Random(a.seed).GetInt() + index).GetInt();
	int seed_2 = Random(Random(b.seed).GetInt() + index).GetInt();
	int combined_seed(seed_1 + seed_2);
	Random rnd(combined_seed);

	Params params { a, b, index, depth, Random(combined_seed) };
	
	geom::uni::Vector3 result;
	if (depth >= planet_shader_depth_medium)
	{
		CalcMidPointPos_SimpleInterp(polyhedron, result, params);
	}
	else 
	{
		CalcMidPointPos_Random(polyhedron, result, params);
	}
	
	mid_point.pos = static_cast<form::Point::Vector3>(result);
//	mid_point.col = gfx::Color4b::White();

	debug::MarkNodePoint(a, mid_point, 0, 0);
	debug::MarkNodePoint(b, mid_point, 3, 3);
	debug::ClampNodePoints();

	return true;
}

// Comes in normalized. Is then given the correct length.
void PlanetShader::CalcRootPointPos(Random & rnd, geom::uni::Vector3 & position) const
{
	geom::uni::Scalar radius = GetRandomHeightCoefficient(rnd);
	position *= radius;
}

geom::uni::Scalar PlanetShader::GetRandomHeightCoefficient(Random & rnd) const
{
	geom::uni::Scalar random_exponent = (.5 - rnd.GetFloatInclusive<geom::uni::Scalar>()) * planet_shader_random_range;
	geom::uni::Scalar coefficient = std::exp(random_exponent);
	return coefficient;
}

// At shallow depth, heigh is highly random.
bool PlanetShader::CalcMidPointPos_Random(form::Polyhedron & polyhedron, geom::uni::Vector3 & result, Params & params) const
{
	auto const & shape = static_cast<geom::uni::Sphere3>(polyhedron.GetShape());
	
	geom::uni::Scalar radius = shape.radius * GetRandomHeightCoefficient(params.rnd);
	polyhedron.GetFormation().SampleRadius(radius);

	geom::uni::Vector3 near_a = GetLocalPosition(params.a.GetCorner(TriMod(params.index + 1))->pos, shape.center);
	geom::uni::Vector3 near_b = GetLocalPosition(params.b.GetCorner(TriMod(params.index + 1))->pos, shape.center);
	result = near_a + near_b;
	geom::uni::Scalar length = Magnitude(result);
	result *= (radius / length);
	result += shape.center;
	
	return true;
}

bool PlanetShader::CalcMidPointPos_SimpleInterp(form::Polyhedron & polyhedron, geom::uni::Vector3 & result, Params & params) const
{
	auto const & shape = static_cast<geom::uni::Sphere3>(polyhedron.GetShape());
	
	geom::uni::Vector3 near_a = GetLocalPosition(params.a.GetCorner(TriMod(params.index + 1))->pos, shape.center);
	geom::uni::Vector3 near_b = GetLocalPosition(params.b.GetCorner(TriMod(params.index + 1))->pos, shape.center);
	result = near_a + near_b;
	geom::uni::Scalar result_length = Magnitude(result);
	
	geom::uni::Scalar near_a_altitude = GetAltitude(near_a);
	geom::uni::Scalar near_b_altitude = GetAltitude(near_b);
	geom::uni::Scalar altitude = (near_a_altitude + near_b_altitude) * .5;

	geom::uni::Scalar rnd_x = params.rnd.GetFloatInclusive<geom::uni::Scalar>() * 2. - 1.;
	rnd_x *= Squared(rnd_x);
	
	// Figure out how much the altitude may be varied in either direction,
	// and clip that variance based on the hard limits of the planet.
	// Actually, clip it to half of that to make it look less like a hard limit.
	// And do the clipping based on how far the variance /might/ go.
	geom::uni::Scalar altitude_variance_coefficient = planet_shader_medium_coefficient / geom::uni::Scalar(1 << params.depth);
	
	geom::uni::Scalar lod_variation_cycler = sin(0.1 * std::max(0, params.depth - planet_shader_depth_medium));
	altitude_variance_coefficient *= lod_variation_cycler;
	
	altitude_variance_coefficient *= shape.radius;
	
	altitude += rnd_x * altitude_variance_coefficient;
	polyhedron.GetFormation().SampleRadius(altitude);
	
	result *= (altitude / result_length);
	result += shape.center;
	
	return true;
}

geom::uni::Vector3 PlanetShader::GetLocalPosition(form::Point const & point, geom::uni::Vector3 const & center) const
{
	return GetLocalPosition(point.pos, center);
}

geom::uni::Vector3 PlanetShader::GetLocalPosition(form::Vector3 const & point_pos, geom::uni::Vector3 const & center) const
{
	return static_cast<geom::uni::Vector3>(point_pos) - center;
}

geom::uni::Scalar PlanetShader::GetAltitude(form::Point const & point, geom::uni::Vector3 const & center) const
{
	return GetAltitude(point.pos, center);
}

geom::uni::Scalar PlanetShader::GetAltitude(form::Vector3 const & point_pos, geom::uni::Vector3 const & center) const
{
	return GetAltitude(GetLocalPosition(point_pos, center));
}

geom::uni::Scalar PlanetShader::GetAltitude(geom::uni::Vector3 const & local_pos) const
{
	return Magnitude(local_pos);
}

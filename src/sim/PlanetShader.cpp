//
//  PlanetShader.cpp
//  crag
//
//  Created by John on 2/21/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Planet.h"
#include "PlanetShader.h"
#include "Engine.h"

#include "form/Formation.h"
#include "form/node/Node.h"
#include "form/node/Point.h"
#include "form/scene/Polyhedron.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

#include "gfx/Color.h"
#include "gfx/Debug.h"


namespace 
{
	
	// Config values
	CONFIG_DEFINE (planet_shader_depth_medium, int, 2);
	CONFIG_DEFINE (planet_shader_random_range, sim::Scalar, 0.002);
	CONFIG_DEFINE (planet_shader_medium_coefficient, sim::Scalar, .35);
	
	sim::Scalar root_three = sqrt(3.);
	
	// RootNode initialization data. A tetrahedron.
	sim::Vector3 root_corners[4] = 
	{
		sim::Vector3(-1,  1,  1),
		sim::Vector3( 1, -1,  1),
		sim::Vector3( 1,  1, -1),
		sim::Vector3(-1, -1, -1)
	};
	
	// InitMidPoint helper
	int MeasureDepth(form::Node const * n, int max_depth)
	{
		int depth = 0;
		
		while (true)
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
#if defined(NDEBUG) || 1
	void ClearNodePoints() { }
	void MarkNodePoint(form::Node const & node, form::Point const & point, int row, int column) { }
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
// sim::PlanetShader::Params

class sim::PlanetShader::Params
{
public:
	Params(form::Node const & init_a, form::Node const & init_b, int init_index, int init_depth, Random init_rnd)
	: a(init_a)
	, b(init_b)
	, index(init_index)
	, depth(init_depth)
	, rnd(init_rnd)
	{
	}
	
	form::Node const & a;
	form::Node const & b;
	int const index;
	int const depth;	// as a proportion of planet_shader_depth_deep
	Random rnd;
};


////////////////////////////////////////////////////////////////////////////////
// PlanetShader

void sim::PlanetShader::InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const
{
	int seed = polyhedron.GetFormation().GetSeed();
	
	// This one progresses with each iteration.
	Random point_randomizer(seed + 1);
	
	// This one is the same each time.
	//Random crater_randomizer(seed + 2);

	Sphere3 shape = polyhedron.GetShape();
	Scalar inverse_root_corner_length = 1. / root_three;
	for (int i = 0; i < 4; ++ i)
	{
		Vector3 position = root_corners[i] * inverse_root_corner_length;
		CalcRootPointPos(point_randomizer, position);
		position *= shape.radius;
		position += shape.center;
		points[i]->pos = position;
	}
}

bool sim::PlanetShader::InitMidPoint(form::Polyhedron & polyhedron, form::Node const & a, form::Node const & b, int index, form::Point & mid_point) const
{
	int max_depth = std::numeric_limits<int>::max();
	int depth = MeasureDepth(& a, max_depth);
	ASSERT(depth == MeasureDepth(& b, max_depth));

	int seed_1 = Random(Random(a.seed).GetInt() + index).GetInt();
	int seed_2 = Random(Random(b.seed).GetInt() + index).GetInt();
	int combined_seed(seed_1 + seed_2);
	Random rnd(combined_seed);

	Params params(a, b, index, depth, combined_seed);
	
	Vector3 result;
	if (depth >= planet_shader_depth_medium)
	{
		CalcMidPointPos_SimpleInterp(polyhedron, result, params);
	}
	else 
	{
		CalcMidPointPos_Random(polyhedron, result, params);
	}
	
	mid_point.pos = result;
//	mid_point.col = gfx::Color4b::White();

	debug::MarkNodePoint(a, mid_point, 0, 0);
	debug::MarkNodePoint(b, mid_point, 3, 3);
	debug::ClampNodePoints();

	return true;
}

// Comes in normalized. Is then given the correct length.
void sim::PlanetShader::CalcRootPointPos(Random & rnd, sim::Vector3 & position) const
{
	Scalar radius = GetRandomHeightCoefficient(rnd);
	position *= radius;
}

sim::Scalar sim::PlanetShader::GetRandomHeightCoefficient(Random & rnd) const
{
	Scalar random_exponent = (.5 - rnd.GetUnitInclusive<Scalar>()) * planet_shader_random_range;
	Scalar coefficient = std::exp(random_exponent);
	return coefficient;
}

// At shallow depth, heigh is highly random.
bool sim::PlanetShader::CalcMidPointPos_Random(form::Polyhedron & polyhedron, sim::Vector3 & result, Params & params) const
{
	Sphere3 const & shape = polyhedron.GetShape();
	
	Scalar radius = shape.radius * GetRandomHeightCoefficient(params.rnd);
	polyhedron.GetFormation().SampleRadius(radius);

	Vector3 near_a = GetLocalPosition(params.a.GetCorner(TriMod(params.index + 1)).pos, shape.center);
	Vector3 near_b = GetLocalPosition(params.b.GetCorner(TriMod(params.index + 1)).pos, shape.center);
	result = near_a + near_b;
	Scalar length = Length(result);
	result *= (radius / length);
	result += shape.center;
	
	return true;
}

bool sim::PlanetShader::CalcMidPointPos_SimpleInterp(form::Polyhedron & polyhedron, sim::Vector3 & result, Params & params) const 
{
	Sphere3 const & shape = polyhedron.GetShape();
	
	Vector3 near_a = GetLocalPosition(params.a.GetCorner(TriMod(params.index + 1)).pos, shape.center);
	Vector3 near_b = GetLocalPosition(params.b.GetCorner(TriMod(params.index + 1)).pos, shape.center);
	result = near_a + near_b;
	Scalar result_length = Length(result);
	
	Scalar near_a_altitude = GetAltitude(near_a);
	Scalar near_b_altitude = GetAltitude(near_b);
	Scalar altitude = (near_a_altitude + near_b_altitude) * .5;

	Scalar rnd_x = params.rnd.GetUnitInclusive<Scalar>() * 2. - 1.;
	rnd_x *= Square(rnd_x);
	
	// Figure out how much the altitude may be varied in either direction,
	// and clip that variance based on the hard limits of the planet.
	// Actually, clip it to half of that to make it look less like a hard limit.
	// And do the clipping based on how far the variance /might/ go.
	Scalar altitude_variance_coefficient = planet_shader_medium_coefficient / Scalar(1 << params.depth);
	
	Scalar lod_variation_cycler = sin(0.1 * std::max(0, params.depth - planet_shader_depth_medium));
	altitude_variance_coefficient *= lod_variation_cycler;
	
	altitude_variance_coefficient *= shape.radius;
	
	altitude += rnd_x * altitude_variance_coefficient;
	polyhedron.GetFormation().SampleRadius(altitude);
	
	result *= (altitude / result_length);
	result += shape.center;
	
	return true;
}

sim::Vector3 sim::PlanetShader::GetLocalPosition(form::Point const & point, Vector3 const & center) const
{
	return GetLocalPosition(point.pos, center);
}

sim::Vector3 sim::PlanetShader::GetLocalPosition(form::Vector3 const & point_pos, Vector3 const & center) const
{
	return sim::Vector3(point_pos) - center;
}

sim::Scalar sim::PlanetShader::GetAltitude(form::Point const & point, Vector3 const & center) const
{
	return GetAltitude(point.pos, center);
}

sim::Scalar sim::PlanetShader::GetAltitude(form::Vector3 const & point_pos, Vector3 const & center) const
{
	return GetAltitude(GetLocalPosition(point_pos, center));
}

sim::Scalar sim::PlanetShader::GetAltitude(sim::Vector3 const & local_pos) const
{
	return Length(local_pos);
}

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
#include "Simulation.h"

#include "form/Formation.h"
#include "form/node/Node.h"
#include "form/node/Point.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"

#include "geom/VectorOps.h"
#include "geom/SphereOps.h"

#include "gfx/Color.h"
#include "gfx/Debug.h"


// TODO: Really big todo list:
// TODO: Try 4-float co-ordinate system: normalized x, y, z and altitude.
// TODO: Divorce heigh variation from near_distance


namespace 
{
	
	// Config values
	CONFIG_DEFINE (planet_shader_depth_medium, int, 2);
	CONFIG_DEFINE (planet_shader_depth_deep, int, 12);
	CONFIG_DEFINE (planet_shader_random_range, sim::Scalar, 0.001);
	CONFIG_DEFINE (planet_shader_medium_coefficient, sim::Scalar, .25);
	
	sim::Scalar root_three = Sqrt(3.);
	
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
	sys::Mutex m;
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
// sim::PlanetShader::PlanetShader

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

sim::PlanetShader::PlanetShader(Planet & init_planet)
: center(Vector3::Zero())
, planet(init_planet)
{
}

void sim::PlanetShader::SetOrigin(Vector3d const & origin)
{
	// Get a lock on the simulation object 
	// as this is likely to be run from a different thread.
	// TODO: This is silly. So is TickModels. Fix it. 
	Simulation::ptr lock(Simulation::GetLock());
	
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
		CalcRootPointPos(point_randomizer, position);
		position += center;
		points[i]->pos = position;
	}
}

bool sim::PlanetShader::InitMidPoint(form::Point & mid_point, form::Node const & a, form::Node const & b, int index) 
{
	int max_depth = std::numeric_limits<int>::max();
	int depth = MeasureDepth(& a, max_depth);
	Assert(depth == MeasureDepth(& b, max_depth));

	int seed_1 = Random(Random(a.seed).GetInt() + index).GetInt();
	int seed_2 = Random(Random(b.seed).GetInt() + index).GetInt();
	int combined_seed(seed_1 + seed_2);
	Random rnd(combined_seed);

	Params params(a, b, index, depth, combined_seed);
	
	Vector3 result;
	if (depth >= planet_shader_depth_medium)
	{
		if (false && depth >= planet_shader_depth_deep)
		{
			if (! CalcMidPointPos_BicubicInterp(result, params))
			{
				return false;
			}
			
			mid_point.col = gfx::Color4b::Red();
		}
		else
		{
			CalcMidPointPos_SimpleInterp(result, params);
			mid_point.col = gfx::Color4b::Blue();
		}
	}
	else 
	{
		CalcMidPointPos_Random(result, params);
		mid_point.col = gfx::Color4b::Green();
	}
	
#if ! defined(NDEBUG)
	// Check that resultant point is within the [min, max] range.
	Scalar altitude = Length(result - center);
	if (altitude < planet.GetRadiusMin() * .99999f)
	{
		Assert(false);
	}
	if (altitude > planet.GetRadiusMax() * 1.00001f)
	{
		Assert(false);
	}
#endif
	
	mid_point.pos = result;
	mid_point.col = gfx::Color4b::White();

	debug::MarkNodePoint(a, mid_point, 0, 0);
	debug::MarkNodePoint(b, mid_point, 3, 3);
	debug::ClampNodePoints();

	return true;
}

// Comes in normalized. Is then given the correct length.
void sim::PlanetShader::CalcRootPointPos(Random & rnd, sim::Vector3 & position) const
{
	Scalar radius = GetRandomHeight(rnd);
	position *= radius;
}

sim::Scalar sim::PlanetShader::GetRandomHeight(Random & rnd) const
{
	Scalar radius_mean = planet.GetRadiusMean();
	Scalar random_exponent = (.5 - rnd.GetFloatInclusive()) * planet_shader_random_range;
	Scalar radius = radius_mean * Exp(random_exponent);
	return radius;
}

// At shallow depth, heigh is highly random.
bool sim::PlanetShader::CalcMidPointPos_Random(sim::Vector3 & result, Params & params) const
{
	Scalar radius = GetRandomHeight(params.rnd);
	planet.SampleRadius(radius);

	Vector3 near_a = GetLocalPosition(params.a.GetCorner(TriMod(params.index + 1)).pos);
	Vector3 near_b = GetLocalPosition(params.b.GetCorner(TriMod(params.index + 1)).pos);
	result = near_a + near_b;
	Scalar length = Length(result);
	result *= (radius / length);
	result += center;
	
	return true;
}

bool sim::PlanetShader::CalcMidPointPos_SimpleInterp(sim::Vector3 & result, Params & params) const 
{
	Vector3 near_a = GetLocalPosition(params.a.GetCorner(TriMod(params.index + 1)).pos);
	Vector3 near_b = GetLocalPosition(params.b.GetCorner(TriMod(params.index + 1)).pos);
	result = near_a + near_b;
	Scalar result_length = Length(result);
	
	Scalar near_a_altitude = GetAltitude(near_a);
	Scalar near_b_altitude = GetAltitude(near_b);
	Scalar altitude = (near_a_altitude + near_b_altitude) * .5;

	Scalar rnd_x = params.rnd.GetFloatInclusive() * 2. - 1.;
	rnd_x *= Square(rnd_x);
	
	// Figure out how much the altitude may be varied in either direction,
	// and clip that variance based on the hard limits of the planet.
	// Actually, clip it to half of that to make it look less like a hard limit.
	// And do the clipping based on how far the variance /might/ go.
	Scalar altitude_variance_coefficient = planet_shader_medium_coefficient / static_cast<Scalar>(1 << params.depth);
	
	Scalar lod_variation_cycler = Sin(0.1 * Max(0, params.depth - planet_shader_depth_medium));
	altitude_variance_coefficient *= lod_variation_cycler;
	
	altitude_variance_coefficient *= planet.GetRadiusMean();
	
	altitude += rnd_x * altitude_variance_coefficient;
	planet.SampleRadius(altitude);
	
	result *= (altitude / result_length);
	result += center;
	
	return true;
}

bool sim::PlanetShader::CalcMidPointPos_BicubicInterp(sim::Vector3 & result, Params & params) const 
{
	// Get the grid.
	PointGrid grid;
	
	if (! GetPointGrid(grid, params))
	{
		return false;
	}
	
	Assert(grid[2][1] == & params.a.GetCorner(TriMod(params.index + 1)));
	Assert(grid[1][2] == & params.b.GetCorner(TriMod(params.index + 1)));
	Assert(grid[1][1] == & params.a.GetCorner(params.index));
	Assert(grid[2][2] == & params.b.GetCorner(params.index));
	
	Scalar alts[4][4];
	GridToAltitude(alts, grid);
	Scalar altitude = BicubicInterpolation(alts, .5, .5);
	
	Vector3 near_a = GetLocalPosition(* grid[2][1]);
	Vector3 near_b = GetLocalPosition(* grid[1][2]);
	Scalar near_distance = Length(near_a - near_b);

	result = near_a + near_b;
	Scalar result_length = Length(result);
	
	Scalar rnd_x = params.rnd.GetFloatInclusive() * 2. - 1.;
	rnd_x *= Square(rnd_x);
	
	// Figure out how much the altitude may be varied in either direction,
	// and clip that variance based on the hard limits of the planet.
	// Actually, clip it to half of that to make it look less like a hard limit.
	// And do the clipping based on how far the variance /might/ go.
	Scalar altitude_variance_coefficient = near_distance * planet_shader_medium_coefficient;
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
	
	result *= (altitude / result_length);
	return true;
}

bool sim::PlanetShader::GetPointGrid(PointGrid & grid, Params const & params)
{
	form::Node const * lattice [3][3][2];
	GetNodeLattice(lattice, params);

	if (! LatticeToGrid(grid, lattice, params.index))
	{
		return false;
	}

	return true;
}

// Two nodes, a and b, which are cousins are given; their common side has the given index.
// Toegether, they form a quadrilateral. That quadrilateral is the center of a 3x3 grid, lattice.
// The function makes a decent attempt at returning all the nodes in that lattice.
void sim::PlanetShader::GetNodeLattice(NodeLattice & lattice, Params const & params)
{
	ZeroMemory(reinterpret_cast<char *>(lattice), sizeof(form::Node *) * 3 * 3 * 2);
	lattice [1][1][0] = & params.a;
	lattice [1][1][1] = & params.b;
	
	int row_index = TriMod(params.index + 1);
	int column_index = TriMod(params.index + 2);
	
	while (true)
	{
		bool changed = false;
		
		for (int row = 0; row < 3; ++ row)
		{
			for (int column = 0; column < 3; ++ column)
			{
				for (int node_index = 0; node_index < 2; ++ node_index)
				{
					// For each node in the lattice,
					form::Node const * n = lattice[row][column][node_index];				
					if (n != nullptr)
					{
						// we've already got this one so yay!
						continue;
					}
					// Otherwise, there are three (easy) ways to get the pointer we're after:
					
					// 1: The 'buddy'.
					int buddy_index = ! node_index;
					form::Node const * buddy = lattice[row][column][buddy_index];
					if (buddy != nullptr)
					{
						n = buddy->GetCousin(params.index);
						if (n != nullptr)
						{
							lattice[row][column][node_index] = n;
							
							// Node was retrieved from his buddy.
							changed = true;
							continue;
						}
					}
					
					// 2: From the 'row neighbour'.
					form::Node const * row_neighbour = nullptr;					
					if (node_index == 0)
					{
						if (row > 0)
						{
							row_neighbour = lattice[row - 1][column][buddy_index];
						}
					}
					else
					{
						if (row < 2)
						{
							row_neighbour = lattice[row + 1][column][buddy_index];
						}
					}
					
					if (row_neighbour != nullptr)
					{
						n = row_neighbour->GetCousin(row_index);
						if (n != nullptr)
						{
							lattice[row][column][node_index] = n;
							// Node was retrieved from his row neighbour.
							changed = true;
							continue;
						}
					}
					
					// 3: From the 'column neighbour'.
					form::Node const * column_neighbour = nullptr;					
					if (node_index == 0)
					{
						if (column > 0)
						{
							column_neighbour = lattice[row][column - 1][buddy_index];
						}
					}
					else
					{
						if (column < 2)
						{
							column_neighbour = lattice[row][column + 1][buddy_index];
						}
					}
					
					if (column_neighbour != nullptr)
					{
						n = column_neighbour->GetCousin(column_index);
						if (n != nullptr)
						{
							lattice[row][column][node_index] = n;
							
							// Node was retrieved from his row neighbour.
							changed = true;
							continue;
						}
					}
				}
			}
		}
		
		// Did we get anything done this pass?
		if (! changed)
		{
			break;
		}
	}
}

// Scans through the given lattice of nodes and uses their corner points to fill in the grid of points.
// The same corner point may be shared by up to six nodes. 
bool sim::PlanetShader::LatticeToGrid(PointGrid & grid, NodeLattice const & lattice, int index)
{
	int index_1 = TriMod(index + 1);
	int index_2 = TriMod(index + 2);
	
	debug::ClearNodePoints();
	
	ZeroMemory(reinterpret_cast<char *>(grid), sizeof(form::Point const *) * 4 * 4);
	for (int row = 0; row < 4; ++ row)
	{
		for (int column = 0; column < 4; ++ column)
		{
			// Try and get the point from one of the nodes...
			form::Node const * node;
			form::Point const * point;
			
			// 1: the top-left corner of the first node.
			if (row < 3 && column < 3)
			{
				node = lattice[row][column][0];
				if (node != nullptr)
				{
					point = node->GetCornerPtr(index);
					if (point != nullptr)
					{
						grid[row][column] = point;
						debug::MarkNodePoint(* node, * point, row, column);
						continue;
					}
				}
			}
			
			// 2: the bottom-right corner of the second node.
			if (row > 0 && column > 0)
			{
				node = lattice[row - 1][column - 1][1];
				if (node != nullptr)
				{
					point = node->GetCornerPtr(index);
					if (point != nullptr)
					{
						grid[row][column] = point;
						debug::MarkNodePoint(* node, * point, row, column);
						continue;
					}
				}
			}
			
			// 3: top-right corner - there's two ways to this one...
			if (row < 3 && column > 0)
			{
				node = lattice[row][column - 1][0];
				if (node != nullptr)
				{
					point = node->GetCornerPtr(index_2);
					if (point != nullptr)
					{
						grid[row][column] = point;
						debug::MarkNodePoint(* node, * point, row, column);
						continue;
					}
				}
				
				node = lattice[row][column - 1][1];
				if (node != nullptr)
				{
					point = node->GetCornerPtr(index_1);
					if (point != nullptr)
					{
						grid[row][column] = point;
						debug::MarkNodePoint(* node, * point, row, column);
						continue;
					}
				}
			}
			
			// 4: bottom-left corner - there's two ways to this one too...
			if (row > 0 && column < 3)
			{
				node = lattice[row - 1][column][0];
				if (node != nullptr)
				{
					point = node->GetCornerPtr(index_1);
					if (point != nullptr)
					{
						grid[row][column] = point;
						debug::MarkNodePoint(* node, * point, row, column);
						continue;
					}
				}
				
				node = lattice[row - 1][column][1];
				if (node != nullptr)
				{
					point = node->GetCornerPtr(index_2);
					if (point != nullptr)
					{
						grid[row][column] = point;
						debug::MarkNodePoint(* node, * point, row, column);
						continue;
					}
				}
			}
			
			// 5: Fail
			debug::ClearNodePoints();
			return false;
		}
	}
	
	return true;
}

void sim::PlanetShader::GridToAltitude(Scalar altitude[4][4], PointGrid const & grid) const
{
	for (int row = 0; row < 4; ++ row)
	{
		for (int column = 0; column < 4; ++ column)
		{
			altitude[row][column] = GetAltitude(* grid[row][column]);
		}
	}
}

sim::Vector3 sim::PlanetShader::GetLocalPosition(form::Point const & point) const
{
	return GetLocalPosition(point.pos);
}

sim::Vector3 sim::PlanetShader::GetLocalPosition(form::Vector3 const & point_pos) const
{
	return sim::Vector3(point_pos) - center;
}

sim::Scalar sim::PlanetShader::GetAltitude(form::Point const & point) const
{
	return GetAltitude(point.pos);
}

sim::Scalar sim::PlanetShader::GetAltitude(form::Vector3 const & point_pos) const
{
	return GetAltitude(GetLocalPosition(point_pos));
}

sim::Scalar sim::PlanetShader::GetAltitude(sim::Vector3 const & local_pos) const
{
	return Length(local_pos);
}


////////////////////////////////////////////////////////////////////////////////
// PlanetShaderFactory

sim::PlanetShaderFactory::PlanetShaderFactory(Planet & init_planet)
: planet(init_planet)
{
}

form::Shader * sim::PlanetShaderFactory::Create(form::Formation const & formation) const
{
	return new PlanetShader(planet);
}

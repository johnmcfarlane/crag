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


// TODO: http://www.paulinternet.nl/?page=bicubic


namespace 
{
	
	// Config values
	CONFIG_DEFINE (planet_shader_depth_medium, int, 3);
	CONFIG_DEFINE (planet_shader_depth_deep, int, 6);
	CONFIG_DEFINE (planet_shader_error_co, double, 0.995);
	//CONFIG_DEFINE (formation_color, gfx::Color4f, gfx::Color4f(1.f, 1.f, 1.f));
	
	sim::Scalar root_three = Sqrt(3.);
	
	// RootNode initialization data. A tetrahedron.
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

bool sim::PlanetShader::InitMidPoint(form::Point & mid_point, form::Node const & a, form::Node const & b, int index) 
{
	int depth = MeasureDepth(& a, planet_shader_depth_deep);
	Assert(depth == MeasureDepth(& b, planet_shader_depth_deep));
	
	Params params;

	if (! GetGrid(params.grid, a, b, index))
	{
		//Assert(false);
		return false;
	}

	Assert(params.grid[2][1] == & a.GetCorner(TriMod(index + 1)));
	Assert(params.grid[1][2] == & b.GetCorner(TriMod(index + 1)));
	Assert(params.grid[1][1] == & a.GetCorner(index));
	Assert(params.grid[2][2] == & b.GetCorner(index));
	
	params.depth = static_cast<Scalar> (depth) / planet_shader_depth_deep;
	Assert(params.depth >= 0 && params.depth <= 1);
	
	int seed_1 = Random(a.seed + index).GetInt();
	int seed_2 = Random(b.seed + index).GetInt();
	int combined_seed(seed_1 + seed_2);
	Random rnd(combined_seed);
	params.rnd = Random(combined_seed);
	
	Vector3 result;
	if (false && depth >= planet_shader_depth_medium)
	{
		if (false && depth >= planet_shader_depth_deep)
		{
			//result = CalcMidPointPos_Deep(params);
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

	return true;
}

// Comes in normalized. Is then given the correct length.
void sim::PlanetShader::CalcRootPointPointPos(Random & rnd, sim::Vector3 & position) const
{
	Scalar radius = planet.GetRadiusAverage();
	position *= radius;
}

bool sim::PlanetShader::GetGrid(form::Point const * grid[4][4], form::Node const & a, form::Node const & b, int index)
{
	form::Node const * lattice [3][3][2];
	GetNodeLattice(lattice, a, b, index);
	
	int index_1 = TriMod(index + 1);
	int index_2 = TriMod(index + 2);

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
					point = node->triple[index].corner;
					if (point != nullptr)
					{
						grid[row][column] = point;
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
					point = node->triple[index].corner;
					if (point != nullptr)
					{
						grid[row][column] = point;
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
					point = node->triple[index_1].corner;
					if (point != nullptr)
					{
						grid[row][column] = point;
						continue;
					}
				}
				
				node = lattice[row][column - 1][1];
				if (node != nullptr)
				{
					point = node->triple[index_2].corner;
					if (point != nullptr)
					{
						grid[row][column] = point;
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
					point = node->triple[index_2].corner;
					if (point != nullptr)
					{
						grid[row][column] = point;
						continue;
					}
				}
				
				node = lattice[row - 1][column][1];
				if (node != nullptr)
				{
					point = node->triple[index_1].corner;
					if (point != nullptr)
					{
						grid[row][column] = point;
						continue;
					}
				}
			}
			
			// 5: Fail
			//return false;
		}
	}
	
	return true;
}

// Two nodes, a and b, which are cousins are given; their common side has the given index.
// Toegether, they form a quadrilateral. That quadrilateral is the center of a 3x3 grid, lattice.
// The function makes a decent attempt at returning all the nodes in that lattice.
void sim::PlanetShader::GetNodeLattice(form::Node const * lattice [3][3][2], form::Node const & a, form::Node const & b, int index)
{
	ZeroMemory(reinterpret_cast<char *>(lattice), sizeof(form::Node *) * 3 * 3 * 2);
	lattice [1][1][0] = & a;
	lattice [1][1][1] = & b;
	
	int row_index = TriMod(index + 1);
	int column_index = TriMod(index + 2);
	
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
						n = buddy->triple[index].cousin;
						if (n != nullptr)
						{
							lattice[row][column][node_index] = n;
							
							// Node was retrieved from his buddy.
							++ changed;
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
						n = row_neighbour->triple[row_index].cousin;
						if (n != nullptr)
						{
							lattice[row][column][node_index] = n;
							// Node was retrieved from his row neighbour.
							++ changed;
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
						n = column_neighbour->triple[column_index].cousin;
						if (n != nullptr)
						{
							lattice[row][column][node_index] = n;
							
							// Node was retrieved from his row neighbour.
							++ changed;
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

void sim::PlanetShader::GridToAltitude(Scalar altitude[4][4], form::Point const * grid[4][4]) const
{
	for (int row = 0; row < 4; ++ row)
	{
		for (int column = 0; column < 4; ++ column)
		{
			altitude[row][column] = GetAltitude(* grid[row][column]);
		}
	}
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
	
	Vector3 a = GetLocalPosition(* params.grid[1][2]);
	Vector3 b = GetLocalPosition(* params.grid[2][1]);
	Vector3 v = (a + b) * .5;
	Scalar length = Length(v);
	v *= (radius / length);
	
	return v;
}

sim::Vector3 sim::PlanetShader::CalcMidPointPos_Medium(Params & params) const 
{
	Vector3 near_a = GetLocalPosition(* params.grid[1][2]);
	Vector3 near_b = GetLocalPosition(* params.grid[2][1]);
	Vector3 far_a = GetLocalPosition(* params.grid[1][1]);
	Vector3 far_b = GetLocalPosition(* params.grid[2][2]);
	
	Scalar near_a_altitude = GetAltitude(near_a);
	Scalar near_b_altitude = GetAltitude(near_b);
	Scalar far_a_altitude = GetAltitude(far_a);
	Scalar far_b_altitude = GetAltitude(far_b);

	/*Scalar altitude = 0;
	Scalar alts[16], * alts_iterator = alts;
	for (int row = 0; row < 4; ++ row)
	{
		for (int column = 0; column < 4; ++ column)
		{
			* alts_iterator = GetAltitude(* params.grid[column][row]);
			altitude += * alts_iterator;
			++ alts_iterator;
		}
	}
	altitude /= 16;
	int i = params.rnd.GetInt(16);
	altitude = GetAltitude(* params.grid[0][i]);*/
		
	Scalar near_distance = Length(near_a - near_b);
	
//	Scalar far_weight = 0;//near_distance / far_distance;
//	Scalar far_weight_2 = far_weight * 2.;
//	Scalar altitude = (near_a_altitude + near_b_altitude + far_weight * (far_a_altitude + far_b_altitude)) * (1. / (2. + far_weight_2));
	//Scalar altitude = (near_a_altitude + near_b_altitude + far_a_altitude + far_b_altitude) * .25;
	Scalar altitude = (near_a_altitude + near_b_altitude /*+ far_a_altitude + far_b_altitude*/) * .5;
	Vector3 directional = near_a + near_b;
	Scalar directional_length = Length(directional);
		
	Scalar rnd_x = params.rnd.GetFloatInclusive() * 2. - 1.;
	rnd_x *= Square(rnd_x);
	
	// Figure out how much the altitude may be varied in either direction,
	// and clip that variance based on the hard limits of the planet.
	// Actually, clip it to half of that to make it look less like a hard limit.
	// And do the clipping based on how far the variance /might/ go.
#if 0
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
#endif
	
//	altitude += rnd_x * altitude_variance_coefficient;
	
	directional *= (altitude / directional_length);
	return directional;
}

// TODO: Maybe avoid doing the -/+ center by crossing near and far?
/*sim::Vector3 sim::PlanetShader::CalcMidPointPos_Deep(Params & params) const 
{
	Vector3 inter_near = * params.grid[1][2] - * params.grid[2][1];
	Vector3 inter_far = * params.grid[1][1] - * params.grid[2][2];
	Vector3 outward = CrossProduct(inter_far, inter_near);
	Scalar outward_length = Length(outward);
	
	Scalar random = (static_cast<Scalar>(params.rnd.GetFloatInclusive()) - .5) * Length(inter_near) * .05;
	
	sim::Vector3 avg = (* params.grid[1][2] + * params.grid[2][1]) * .5;
	avg += outward * (random / outward_length);
	return avg;
}*/

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

sim::PlanetShaderFactory::PlanetShaderFactory(Planet const & init_planet)
: planet(init_planet)
{
}

form::Shader * sim::PlanetShaderFactory::Create(form::Formation const & formation) const
{
	return new PlanetShader(planet);
}

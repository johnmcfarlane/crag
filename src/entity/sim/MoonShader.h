//
//  MoonShader.h
//  crag
//
//  Created by John on 8/9/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/Shader.h"

#include "geom/Space.h"

#include "core/Random.h"

namespace sim
{
	// forward declarations
	class Planet;
	
	// A class which governs how the edges of individual polys of a moon's surface are sub-divided.
	class MoonShader : public form::Shader
	{
	public:
		MoonShader(int seed, int num_craters, geom::abs::Scalar radius);
		
	private:
		void InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const override;
		bool InitMidPoint(form::Polyhedron & polyhedron, form::Node const & a, form::Node const & b, int index, form::Point & mid_point) const override;
		
		void ApplyCraters(Random rnd, geom::abs::Vector3 & position) const;
		void GenerateCreater(Random & rnd, geom::abs::Sphere3 & crater, geom::abs::Scalar moon_radius) const;
		
		typedef std::vector<geom::abs::Sphere3> CraterVector;
		CraterVector craters;
	};
}

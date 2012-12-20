//
//  MoonShader.h
//  crag
//
//  Created by John on 8/9/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "axes.h"

#include "form/node/Shader.h"

#include "core/Random.h"


namespace sim
{
	// forward declarations
	class Planet;
	
	
	// A class which governs how the edges of individual polys of a moon's surface are sub-divided.
	class MoonShader : public form::Shader
	{
	public:
		MoonShader(int seed, int num_craters, axes::ScalarAbs radius);
		
	private:
		void InitRootPoints(form::Polyhedron & polyhedron, form::Point * points[]) const override;
		bool InitMidPoint(form::Polyhedron & polyhedron, form::Node const & a, form::Node const & b, int index, form::Point & mid_point) const override;
		
		void ApplyCraters(Random rnd, axes::VectorAbs & position) const;
		void GenerateCreater(Random & rnd, axes::SphereAbs & crater, axes::ScalarAbs moon_radius) const;
		
		typedef std::vector<axes::SphereAbs> CraterVector;
		CraterVector craters;
	};
}

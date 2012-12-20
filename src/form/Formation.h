//
//  Formation.h
//  crag
//
//  Created by john on 6/2/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/defs.h"
#include "sim/axes.h"

#include "geom/Sphere.h"


DECLARE_CLASS_HANDLE(sim, Planet);	// sim::PlanetHandle


namespace form 
{
	// forward-declarations
	class Mesh;
	class Node;
	class Shader;
	

	// A formation is an individual element of the formation system.
	// It contains all the data necessary to create a positioned polyhedron.
	class Formation
	{
	public:
		Formation(int seed, Shader const & shader, axes::SphereAbs const & shape, sim::PlanetHandle const & planet);
		~Formation();

		Shader const & GetShader() const;
		axes::SphereAbs const & GetShape() const;	// global coordinate
		int GetSeed() const;
		
		void SendRadiusUpdateMessage() const;		
		void SampleRadius(axes::ScalarAbs sample_radius);

	private:
		int _seed;	// TODO: This needs its own type.
		Shader const & _shader;
		axes::SphereAbs _shape;
		sim::PlanetHandle _planet;
		axes::ScalarAbs _radius_min;
		axes::ScalarAbs _radius_max;
	};

}

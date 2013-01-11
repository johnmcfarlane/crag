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

#include "geom/origin.h"
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
		Formation(int seed, Shader const & shader, geom::abs::Sphere3 const & shape, sim::PlanetHandle const & planet);
		~Formation();

		Shader const & GetShader() const;
		geom::abs::Sphere3 const & GetShape() const;	// global coordinate
		int GetSeed() const;
		
		void SendRadiusUpdateMessage() const;		
		void SampleRadius(geom::abs::Scalar sample_radius);

	private:
		int _seed;
		Shader const & _shader;
		geom::abs::Sphere3 _shape;
		sim::PlanetHandle _planet;
		geom::abs::Scalar _radius_min;
		geom::abs::Scalar _radius_max;
	};

}

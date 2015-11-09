//
//  Formation.h
//  crag
//
//  Created by john on 6/2/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Space.h"
#include "geom/Sphere.h"

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
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		using ShaderPtr = std::shared_ptr<Shader const>;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Formation(Formation const &) = default;
		Formation(int seed, ShaderPtr const & shader, geom::uni::Sphere3 const & shape);

		Formation & operator=(Formation const & rhs) = default;

		Shader const & GetShader() const;
		geom::uni::Sphere3 const & GetShape() const;	// global coordinate
		int GetSeed() const;
		
		void SampleRadius(geom::uni::Scalar sample_radius);
		geom::uni::Scalar GetMaxRadius() const;

	private:
		int _seed;
		ShaderPtr _shader;
		geom::uni::Sphere3 _shape;
		geom::uni::Scalar _max_radius;
	};

}

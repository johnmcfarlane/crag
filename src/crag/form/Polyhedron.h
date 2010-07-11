/*
 *  Image.h
 *  Crag
 *
 *  Created by John on 2/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "RootNode.h"


// TODO: Rename Polyhedron to Polyhedron?

namespace form
{
	Polyhedron & GetModel(Node & node);
	
	class VertexBuffer;
	
	// A Polyhedron is an instance of a Formation.
	// The same Formation is represented in different Scenes by a different Polyhedron.
	// The main difference between different Models of the same formation 
	// is that that have a different root_node and likely live in a different coordinate space.
	class Polyhedron
	{
	public:
		Polyhedron();
		Polyhedron(Polyhedron const & rhs);
		~Polyhedron();
		
		void SetShader(class Shader * init_shader);
		Shader & GetShader();
		
		enum 
		{
			NUM_ROOT_VERTICES = 4
		};
		
		//class Vertex * corners [NUM_ROOT_VERTICES];
		RootNode root_node;	// Exists purely so that all 'real' nodes have a parent.
		Shader * shader;
	};
}

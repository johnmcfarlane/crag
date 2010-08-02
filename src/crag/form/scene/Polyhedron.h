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

#include "form/node/RootNode.h"

#include "sim/defs.h"


namespace form
{
	// Forward-declarations
	class Shader;
	class Formation;
	
	// Helper function; Given a decendant node, find it's root, and then its polyhedron.
	Polyhedron & GetPolyhedron(Node & node);
	
	// A Polyhedron is an instance of a Formation.
	// The same Formation is represented in different Scenes by a different Polyhedron.
	// The main difference between different Models of the same formation 
	// is that that have a different root_node and likely live in a different coordinate space.
	class Polyhedron
	{
		friend Polyhedron & GetPolyhedron(Node & node);
	public:
		Polyhedron();
		Polyhedron(Polyhedron const & rhs);
		~Polyhedron();
		
		//RootNode const & GetRootNode() const;
		Shader & GetShader();
		
		void Init(Formation const & formation, sim::Vector3 const & origin, PointBuffer & point_buffer);
		void Deinit(PointBuffer & point_buffer);

		void SetOrigin(sim::Vector3 const & origin, PointBuffer & point_buffer);
	private:
		bool ResetOrigin(Node & node, PointBuffer & point_buffer, int depth);

		enum 
		{
			NUM_ROOT_VERTICES = 4
		};
		
	public:
		RootNode root_node;	// Exists purely so that all 'real' nodes have a parent.
		Shader * shader;
	};
}

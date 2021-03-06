//
//  Polyhedron.h
//  crag
//
//  Created by John on 2/23/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/Node.h"

#include "geom/Space.h"

namespace form
{
	// Forward-declarations
	class Formation;
	
	// Helper function; Given a decendant node, find it's root, and then its polyhedron.
	Polyhedron * GetPolyhedron(Node & node);
	
	// A Polyhedron is an instance of a Formation.
	// The same Formation is represented in different Scenes by a different Polyhedron.
	// The main difference between different Models of the same formation 
	// is that that have a different root_node and likely live in a different coordinate space.
	class Polyhedron
	{
	public:
		Polyhedron(Formation & formation);
		Polyhedron(Polyhedron const & rhs);
		~Polyhedron();

		Polyhedron & operator=(Polyhedron const &) = delete;
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Polyhedron);
		
		void Init(geom::Space const & space, PointBuffer & point_buffer);
		void Deinit(PointBuffer & point_buffer);

		geom::uni::Sphere3 const & GetShape() const;
		Formation & GetFormation();
		Formation const & GetFormation() const;
		Node const & GetRootNode() const;
		
		void SetSpace(geom::Space const & space);
	private:
		bool ResetSpace(Node & node, PointBuffer & point_buffer, int depth);
		
		enum 
		{
			NUM_ROOT_VERTICES = 4
		};
		
	public:
		geom::uni::Sphere3 _shape;
		Node _root_node;	// Exists purely so that all 'real' nodes have a parent.
		Formation & _formation;
	};
}

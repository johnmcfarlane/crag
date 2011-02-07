/*
 *  GenerateMeshFunctor.h
 *  Crag
 *
 *  Created by John on 3/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Node.h"

#include "form/scene/Mesh.h"
#include "ForEachNodeFace.h"


namespace form 
{
	// forward-declarations
	class Mesh;
	

	// This node functor is called on every used node in the NodeNuffer and
	// uses the vertices associated with those nodes to generate the given mesh.
	class GenerateMeshFunctor
	{
		OBJECT_NO_COPY (GenerateMeshFunctor);

	public:
		GenerateMeshFunctor(Mesh & _mesh) 
		: mesh(_mesh) 
		{ 
		}
		
		// NodeBuffer::ForEachNode functor
		void operator() (Node & node)
		{
			if (node.IsLeaf()) 
			{
				ForEachNodeFace(node, * this);
			}
		}
		
		// ForEachNodeFact functor
		void operator()(Point & a, Point & b, Point & c, Vector3f const & normal)
		{
			mesh.AddFace(a, b, c, normal);
		}
		
	private:
		
		Mesh & mesh;
	};

}

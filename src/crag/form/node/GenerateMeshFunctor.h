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
		
		void operator() (Node & node)
		{
			if (node.IsLeaf()) 
			{
				ForEachNodeFace(node, mesh);
			}
		}
		
	private:
		
		Mesh & mesh;
	};

	
	// This node functor is called prior to GenerateMeshFunctor 
	// to prefetch the points associated with a node.
	void GenerateMeshPrefetchFunctor (Node & node)
	{
		if (node.IsLeaf()) 
		{
			PrefetchBlock(reinterpret_cast<void const *>(& node.GetCorner(0)));
			PrefetchBlock(reinterpret_cast<void const *>(& node.GetCorner(1)));
			PrefetchBlock(reinterpret_cast<void const *>(& node.GetCorner(2)));
			// TODO: Consider/test prefetching mid_points. 
		}
	}

}

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

#include "NodeFunctor.h"

#include "Node.h"
#include "ForEachNodeFace.h"
#include "form/scene/Mesh.h"

namespace form 
{
	// forward-declarations
	class Mesh;
	

	// This node functor is called on every used node in the NodeNuffer and
	// uses the vertices associated with those nodes to generate the given mesh.
	class GenerateMeshFunctor : public NodeFunctor
	{
	public:
		GenerateMeshFunctor(Mesh & _mesh) 
		: mesh(_mesh) 
		{ 
		}
		
		bool IsLeaf(Node const & node) const
		{
			return /*node.IsInUse() &&*/ ! node.HasChildren();
		}
		
		bool PerformPrefetchPass() const
		{
			return true;
		}
		
		void OnPrefetchPass(Node const & node)
		{
			if (IsLeaf(node)) 
			{
				PrefetchBlock(reinterpret_cast<void const *>(& node.GetCorner(0)));
				PrefetchBlock(reinterpret_cast<void const *>(& node.GetCorner(1)));
				PrefetchBlock(reinterpret_cast<void const *>(& node.GetCorner(2)));
			}
		}
		
		void operator()(Node & node)
		{
			if (IsLeaf(node)) 
			{
				ForEachNodeFace(node, mesh);
			}
		}
		
	private:
		
		Mesh & mesh;
	};

}

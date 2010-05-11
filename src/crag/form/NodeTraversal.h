/*
 *  NodeTraversal.h
 *  Crag
 *
 *  Created by John on 11/4/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#include "core/debug.h"
#include "form/Node.h"

namespace form
{
	// Can provide base class for otherwise-incomplete custom functor,
	// or simply provide example of how to write a functor for ForEachPoly.
	class NodeTraversalFunctor
	{
	public:
		// MUST return false if no children.
		bool OnEnterNode(Node & node, int /*depth*/)
		{
			return node.children != nullptr;
		}
		
		void OnExitNode(Node & /*stub*/, int /*depth*/)
		{
		}
	};


	template<typename Functor> void ForEachNode(Node & node, Functor & f, int depth = 0) 
	{
		if (f.OnEnterNode(node, depth)) {
			Node * child = node.children;
			Assert (child != nullptr);
			Node const * const children_end = child + 4;

			++ depth;
			do {
				ForEachNode(* child, f, depth);
				++ child;
			}	while (child != children_end);
		}
		
		f.OnExitNode(node, depth);
	}
}

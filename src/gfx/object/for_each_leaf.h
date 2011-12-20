//
//  for_each_leaf.h
//  crag
//
//  Created by John McFarlane on 2011-11-23.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "LeafNode.h"
#include "BranchNode.h"


namespace gfx
{
	////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////
	// for_each_leaf - scene graph traversal
	
	////////////////////////////////////////////////////////////////////////////////
	// simplified helper functions; call these with root node
	
	template <typename FUNCTOR>
	void for_each_leaf(BranchNode & branch_node, FUNCTOR functor)
	{
		Transformation const & root_transformation = branch_node.GetTransformation();
		for_each_leaf<FUNCTOR, Object, LeafNode, BranchNode, Object::ChildList, Object::ChildList::iterator>(branch_node, functor, root_transformation);
	}
	
	template <typename FUNCTOR>
	void for_each_leaf(BranchNode const & branch_node, FUNCTOR functor)
	{
		Transformation const & root_transformation = branch_node.GetTransformation();
		for_each_leaf<FUNCTOR, Object const, LeafNode const, BranchNode const, Object::ChildList const, Object::ChildList::const_iterator>(branch_node, functor, root_transformation);
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// main recursive functions; do not call directly
	
	template <typename FUNCTOR, typename OBJECT, typename LEAF_NODE, typename BRANCH_NODE, typename LIST_OBJECT, typename CHILD_LIST_ITERATOR>
	void for_each_leaf(BRANCH_NODE & branch_node, FUNCTOR functor, Transformation const & model_view_transformation)
	{
		Transformation scratch;
		
		for (CHILD_LIST_ITERATOR i = branch_node.Begin(), end = branch_node.End(); i != end; ++ i)
		{
			OBJECT & child = * i;
			if (! functor(child))
			{
				continue;
			}
			
			Transformation const & child_model_view_transformation = child.Transform(model_view_transformation, scratch);

			switch (child.GetNodeType())
			{
				default:
					Assert(false);
				case Object::leaf:
				{
					LEAF_NODE & child_leaf = child.CastLeafNodeRef();

					functor(child_leaf, child_model_view_transformation);
					break;
				}
					
				case Object::branch:
				{
					BRANCH_NODE & child_branch = child.CastBranchNodeRef();
					
					for_each_leaf<FUNCTOR, OBJECT, LEAF_NODE, BRANCH_NODE, LIST_OBJECT, CHILD_LIST_ITERATOR>(child_branch, functor, child_model_view_transformation);
					break;
				}
			}
		}
	}
}

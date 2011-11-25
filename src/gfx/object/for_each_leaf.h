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
	template <typename FUNCTOR, typename OBJECT, typename LEAF_NODE, typename BRANCH_NODE, typename LIST_OBJECT, typename CHILD_LIST_ITERATOR>
	void for_each_leaf(BRANCH_NODE & branch_node, FUNCTOR functor)
	{
		for (CHILD_LIST_ITERATOR i = branch_node.Begin(), end = branch_node.End(); i != end; ++ i)
		{
			OBJECT & child = * i;
			if (! functor(child))
			{
				continue;
			}
			
			if (child.GetNodeType() == Object::leaf)
			{
				LEAF_NODE & leaf = child.GetLeafNodeRef();
				functor(leaf);
			}
			else
			{
				Assert(child.GetNodeType() == Object::branch);

				BRANCH_NODE & branch = child.GetBranchNodeRef();
				if (functor(branch))
				{
					for_each_leaf<FUNCTOR, OBJECT, LEAF_NODE, BRANCH_NODE, LIST_OBJECT, CHILD_LIST_ITERATOR>(branch, functor);
				}
			}
		}
	}
	
	template <typename FUNCTOR>
	void for_each_leaf(BranchNode & branch_node, FUNCTOR functor)
	{
		for_each_leaf<FUNCTOR, Object, LeafNode, BranchNode, Object::ChildList, Object::ChildList::iterator>(branch_node, functor);
	}
	
	template <typename FUNCTOR>
	void for_each_leaf(BranchNode const & branch_node, FUNCTOR functor)
	{
		for_each_leaf<FUNCTOR, Object const, LeafNode const, BranchNode const, Object::ChildList const, Object::ChildList::const_iterator>(branch_node, functor);
	}
}

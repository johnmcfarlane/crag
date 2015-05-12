//
//  GatherExpandableNodesFunctor.h
//  crag
//
//  Created by John McFarlane on 2012-04-08.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Node.h"
#include "Surrounding.h"
#include "Quaterna.h"

namespace form
{
	// Called on each node to (conditionally) expand it
	// such that newly allocated nodes are assigned as children of it. 
	class GatherExpandableNodesFunctor
	{
		OBJECT_NO_COPY (GatherExpandableNodesFunctor);
		
	public:
		// functions
		GatherExpandableNodesFunctor (Surrounding & surrounding, NodeVector & expandable_nodes)
		: _surrounding (surrounding)
		, _expandable_nodes(expandable_nodes)
		{
			RecalculateMinScore();
		}
		
		void RecalculateMinScore()
		{
			min_score = _surrounding.GetLowestSortedQuaternaScore();
		}
		
		// The node version. 
		void operator() (Node & node)
		{
			if (node.score > min_score)
			{
				if (node.IsExpandable()) 
				{
					_expandable_nodes.push_back(& node);
				}
			}
		}
		
		// The quaterna version.
		void operator() (Quaterna & quaterna)
		{
			operator() (quaterna.nodes[0]);
			operator() (quaterna.nodes[1]);
			operator() (quaterna.nodes[2]);
			operator() (quaterna.nodes[3]);
		}
		
	private:
		// variables
		Surrounding & _surrounding;
		NodeVector & _expandable_nodes;
		float min_score;
	};
}

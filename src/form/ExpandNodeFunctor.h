//
//  ExpandNodeFunctor.h
//  crag
//
//  Created by John on 3/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
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
	class ExpandNodeFunctor
	{
		OBJECT_NO_COPY (ExpandNodeFunctor);
		
	public:
		// functions
		ExpandNodeFunctor (Surrounding & init_surrounding)
		: surrounding (init_surrounding)
		, num_expanded (0)
		{
			RecalculateMinScore();
		}
		
		void RecalculateMinScore()
		{
			min_score = surrounding.GetLowestSortedQuaternaScore();
		}
		
		int GetNumExpanded() const
		{
			return num_expanded;
		}
		
		// The node version. 
		void operator() (Node * node)
		{
			if (node->score > min_score)
			{
				if (node->IsExpandable()) 
				{
					if (surrounding.ExpandNode(* node)) 
					{
						RecalculateMinScore();
						++ num_expanded;
					}
				}
			}
		}
		
	private:
		// variables
		Surrounding & surrounding;
		int num_expanded;
		float min_score;
	};
}

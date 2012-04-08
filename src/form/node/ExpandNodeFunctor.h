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
#include "NodeBuffer.h"
#include "Quaterna.h"


namespace form 
{
	// Called on each node to (conditionally) expand it
	// such that newly allocated nodes are assigned as children of it. 
	class ExpandNodeFunctor
	{
		OBJECT_NO_COPY (ExpandNodeFunctor);
		
	public:
		ExpandNodeFunctor (NodeBuffer & init_node_buffer)
		: node_buffer (init_node_buffer)
		, num_expanded (0)
		{
			RecalculateMinScore();
		}
		
		void RecalculateMinScore()
		{
			min_score = node_buffer.GetWorseReplacableQuaternaScore();
		}
		
		int GetNumExpanded() const
		{
			return num_expanded;
		}
		
		// The node version. 
		void operator() (Node & node)
		{
			if (node.score > min_score)
			{
				if (node.IsExpandable()) 
				{
					if (node_buffer.ExpandNode(node)) 
					{
						RecalculateMinScore();
						++ num_expanded;
					}
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
		
		NodeBuffer & node_buffer;
		int num_expanded;
		float min_score;
	};
}

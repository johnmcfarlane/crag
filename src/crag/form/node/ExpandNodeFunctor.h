/*
 *  ExpandNodeFunctor.h
 *  Crag
 *
 *  Created by John on 3/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Quaterna.h"

#include "smp/vector.h"


namespace form 
{
	// forward-declarations
	class NodeBuffer;
	
	
	// Called on each node to (conditionally) expand it
	// such that newly allocated nodes are assigned as children of it. 
	class GatherExpandNodeFunctor
	{
		OBJECT_NO_COPY (GatherExpandNodeFunctor);
		
	public:
		GatherExpandNodeFunctor(smp::vector<Node *> & init_expandable_nodes)
		: expandable_nodes(init_expandable_nodes)
		{
		}
		
		// The node version. 
		void operator() (Node & node)
		{
			if (node.IsExpandable()) 
			{
				expandable_nodes.push_back(& node);
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
		smp::vector<Node *> & expandable_nodes;
	};
	
	
	// Called on each node to (conditionally) expand it
	// such that newly allocated nodes are assigned as children of it. 
	class ExpandNodeFunctor
	{
		OBJECT_NO_COPY (ExpandNodeFunctor);
		
	public:
		ExpandNodeFunctor(NodeBuffer & _node_buffer)
		: node_buffer(_node_buffer)
		, num_expanded(0)
		{
		}
		
		int GetNumExpanded() const
		{
			return num_expanded;
		}
		
		// The node version. 
		void operator() (Node * node)
		{
			if (node->IsExpandable()) 
			{
				if (node_buffer.ExpandNode(* node)) 
				{
					++ num_expanded;
				}
			}
		}
		
		// The quaterna version.
		void operator() (Quaterna & quaterna)
		{
			operator() (quaterna.nodes + 0);
			operator() (quaterna.nodes + 1);
			operator() (quaterna.nodes + 2);
			operator() (quaterna.nodes + 3);
		}
		
	private:
		
		NodeBuffer & node_buffer;
		int num_expanded;
	};
}

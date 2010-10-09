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

#include "form/scene/Polyhedron.h"


namespace form 
{
	// forward-declarations
	class NodeBuffer;

	
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
		
		void operator() (Node & node)
		{
			if (node.IsExpandable()) 
			{
				if (node_buffer.ExpandNode(node)) 
				{
					++ num_expanded;
				}
			}
		}
		
	private:

		NodeBuffer & node_buffer;
		int num_expanded;
	};
}

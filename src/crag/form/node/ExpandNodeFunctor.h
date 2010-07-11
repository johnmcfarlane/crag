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

#include "NodeFunctor.h"

#include "Polyhedron.h"


namespace form {
	
	class NodeBuffer;

	class ExpandNodeFunctor : public NodeFunctor
	{
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
		
		bool PerformPrefetchPass() const
		{
			return true;
		}
		
		void OnPrefetchPass(Node const & node) const
		{
			PrefetchBlock(node.parent);
		}
		
		void operator()(Node & node)
		{
			if (node.IsExpandable()) {
				if (node_buffer.ExpandNode(node)) {
					++ num_expanded;
				}
			}
		}
		
	private:

		NodeBuffer & node_buffer;
		int num_expanded;
	};
}

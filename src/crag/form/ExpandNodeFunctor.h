/*
 *  ExpandNodeFunctor.h
 *  Crag
 *
 *  Created by John on 3/29/10.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "NodeFunctor.h"

#include "Model.h"


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
				// TODO: Critical section
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

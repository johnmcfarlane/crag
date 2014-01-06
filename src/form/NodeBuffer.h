//
//  NodeBuffer.h
//  crag
//
//  Created by John on 2/17/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "smp/for_each.h"

namespace form
{
	// forward-declarations
	class Node;
	
	// Node store of variable size with a top limit;
	// used by Surrounding to store all the trees necessary to generate a mesh
	class NodeBuffer
	{
		OBJECT_NO_COPY (NodeBuffer);
	public:
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DECLARE(NodeBuffer);
		void VerifyUsed(Node const & n) const;
		void VerifyUnused(Node const & n) const;
#endif
		
		// Member functions
		NodeBuffer(size_t max_num_nodes);
		~NodeBuffer();
		
		void Clear();
		void Push(std::size_t num_nodes);
		void Pop(std::size_t num_nodes);
		
		void ResetNodeOrigins(Vector3 const & origin_delta);
		
		bool IsEmpty() const;
		std::size_t GetSize() const;
		std::size_t GetCapacity() const;
		Node const & operator[] (std::size_t index) const;
		
		Node const * begin() const;
		Node const * end() const;

		Node * begin();
		Node * end();
		
		template <typename FUNCTOR> 
		void ForEach(FUNCTOR f, size_t step_size, bool parallel)
		{
			if (IsEmpty())
			{
				return;
			}
	
			if (parallel)
			{
				smp::for_each <Node *, FUNCTOR>(_nodes, _nodes_used_end, f, step_size, -1);
			}
			else 
			{
				core::for_each <Node *, FUNCTOR> (_nodes, _nodes_used_end, f);
			}
		}

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		// The fixed-size array of node groups, used and unused.
		Node * const _nodes;	// [max_num_nodes]
		
		Node * _nodes_used_end;			// end of buffer of actually used nodes
		Node const * const _nodes_end;
	};
}

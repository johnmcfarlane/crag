//
//  Quaterna.h
//  crag
//
//  Created by John on 3/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Node.h"


namespace form 
{
	////////////////////////////////////////////////////////////////////////////////
	// Quaterna class
	
	// Used by NodeBuffer class to keep sorted list of nodes in groups of four.
	struct Quaterna
	{
		////////////////////////////////////////////////////////////////////////////////
		// Quaterna members
		
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DECLARE(Quaterna);
#endif
		
		bool IsInUse() const
		{
			return parent_score >= 0;
		}
		
		bool HasGrandChildren() const
		{
			return nodes[0].HasChildren()
				|| nodes[1].HasChildren()
				|| nodes[2].HasChildren()
				|| nodes[3].HasChildren();
		}
		
		bool IsEasilyExpendable() const 
		{
			return ! (IsInUse() && HasGrandChildren());
		}
		
		bool IsSuitableReplacement(float new_parent_score) const
		{
			return parent_score < new_parent_score && ! HasGrandChildren();
		}

		friend void swap(Quaterna & a, Quaterna & b)
		{
			std::swap(a.parent_score, b.parent_score);
			std::swap(a.nodes, b.nodes);
		}

		float parent_score;	// the score of the node for which this is the children
		Node * nodes;	// [4]
	};
	
#if defined(CRAG_VERIFY_ENABLED)
	inline CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Quaterna, self)
		CRAG_VERIFY(self.parent_score);
		CRAG_VERIFY_TRUE(self.nodes != nullptr);
		
		Node * parent = self.nodes[0].GetParent();
		
		if (self.parent_score < 0) {
			CRAG_VERIFY_TRUE(self.parent_score == -1);
			CRAG_VERIFY_TRUE(parent == nullptr);
		}
		else {
			CRAG_VERIFY_TRUE(parent != nullptr);
		}
		
		for (Node * it = self.nodes; it != self.nodes + 4; ++ it) {
			CRAG_VERIFY_TRUE(it->GetParent() == parent);
			CRAG_VERIFY(* it);
		}
	CRAG_VERIFY_INVARIANTS_DEFINE_END
#endif
}

/*
 *  Quaterna.h
 *  Crag
 *
 *  Created by John on 3/29/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

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
		
#if defined(VERIFY)
		void Verify() const
		{
			VerifyTrue(nodes != nullptr);
			
			Node * parent = nodes[0].GetParent();
			
			if (parent_score < 0) {
				VerifyTrue(parent_score == -1);
				VerifyTrue(parent == nullptr);
			}
			else {
				VerifyTrue(parent != nullptr);
			}
			
			for (Node * it = nodes; it != nodes + 4; ++ it) {
				VerifyTrue(it->GetParent() == parent);
				VerifyObject(* it);
			}
		}
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
		
		float parent_score;	// the score of the node for which this is the children
		Node * nodes;	// [4]
	};
	
}

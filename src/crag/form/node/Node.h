/*
 *  Node.h
 *  Crag
 *
 *  Created by john on 5/9/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "form/node/Point.h"

#include "core/memory.h"


namespace form
{
	class Node;
	DUMP_OPERATOR_DECLARATION(Node);

	class Node
	{
	public:
		Node();
		~Node();
		
		bool InitGeometry();
		
		bool HasChildren() const { return children != nullptr; }
		bool IsExpanded() const { return HasChildren(); }
		bool IsRecyclable() const { return ! HasChildren(); }
		bool IsInUse() const { return parent != nullptr; }	// IsInUseOrIsRootNode
		
		bool HasAllCousins() const
		{
			// Make sure all three cousins are available.
			return triple[0].cousin != nullptr && triple[1].cousin != nullptr && triple[2].cousin != nullptr;
		}

		bool IsExpandable() const
		{
			return ! IsExpanded() && HasAllCousins();
		}
		
		Point & GetCorner(int index) { return ref(triple[index].corner); }
		Point const & GetCorner(int index) const { return ref(triple[index].corner); }
		
		Point const * GetMidPoint(int index) const { return triple[index].mid_point; }
		Point * GetMidPoint(int index) { return triple[index].mid_point; }
		
		Node const * GetCousin(int index) const { return triple[index].cousin; }
		Node * GetCousin(int index) { return triple[index].cousin; }
		void SetCousin(int index, Node & cousin);
		
		int GetChildSeed(int child_index) const;
		void GetChildCorners(int child_index, Point * child_corners[3]) const;
		void GetChildNeighbours(int child_index, Node * child_neighbours[3]) const;
		
#if VERIFY
		void Verify() const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(Node);
		
		OVERLOAD_NEW_DELETE(128);
		
		Node * children;		//  4	/	8
		Node * parent;			//  4	/	8
		int seed;				//  4	/	4

		struct Triplet
		{
			Triplet();
			~Triplet();
			
			Point * corner;
			Point * mid_point;
			Node * cousin;
		};	// 12	/	24
		
		Triplet triple[3];		// 36	/	72 72

		Vector3 center;			// 12	/	12
		float area;				//  4	/	 4
		Vector3 normal;			// 12	/	12
		float score;			//  4	/	 4 32
	};	// 80	/
	
}

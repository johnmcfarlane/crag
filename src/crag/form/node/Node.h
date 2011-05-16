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

#include "sys/memory.h"


namespace form
{
	class Node;
	class PointBuffer;
	class Shader;
	
	DUMP_OPERATOR_DECLARATION(Node);

	
	// This is easily one of the most important classes in the formation system. 
	// You can think of it as a triangle with corners represented by the three corner instances.
	
	// This triangle is sub-divided by adding mid-points along its three edges. 
	// These are the three min_point instances.
	// Where all three mid-points are present, the Node can have four children. 
	// Think of the four triangles or the Zelda Triforce, the forth (children[3]) being the upside-down center triangle.
	
	class Node
	{
	public:
		Node();
		~Node();

		// Ensures all three mid-points are allocated and calculated.
		bool InitMidPoints(PointBuffer & point_buffer, Shader & shader);
		static bool InitChildCorners(Node const & parent, Node * children);

		// Calls InitMidPoints and recalcs the center position.
		void Reinit(Shader & shader, PointBuffer & point_buffer);
		
		bool InitScoreParameters();
		
		bool HasChildren() const { return GetChildren() != nullptr; }
		bool IsRecyclable() const { return ! HasChildren(); }
		bool IsInUse() const { return parent != nullptr; }	// IsInUseOrIsRootNode
		bool IsLeaf() const { return ! HasChildren(); }
		
		bool HasAllCousins() const
		{
			// Make sure all three cousins are available.
			//return triple[0].cousin != nullptr && triple[1].cousin != nullptr && triple[2].cousin != nullptr;
			return (size_t(triple[0].cousin) * size_t(triple[1].cousin) * size_t(triple[2].cousin)) != 0;
		}

		bool IsExpandable() const
		{
			Assert(score > 0);
			return ! HasChildren() && HasAllCousins() /*&& score != 0*/;
		}
		
		typedef size_t flag_type;
		static flag_type const flag_mask = 0x3f;
		static flag_type const pointer_mask = ~ flag_mask;
		
		flag_type GetFlags() const { return flags_and_children & flag_mask; }
		void SetFlags(flag_type f);
		
		Node * GetChildren() { return reinterpret_cast<Node *>(flags_and_children & pointer_mask); }
		Node const * GetChildren() const { return reinterpret_cast<Node *>(flags_and_children & pointer_mask); }
		void SetChildren(Node * c);
		
		Node * GetParent() { return parent; }
		Node const * GetParent() const { return parent; }
		void SetParent(Node * p);
		
		Point * GetCornerPtr(int index) { return triple[index].corner; }
		Point const * GetCornerPtr(int index) const { return triple[index].corner; }
		Point & GetCorner(int index) { return ref(GetCornerPtr(index)); }
		Point const & GetCorner(int index) const { return ref(GetCornerPtr(index)); }
		
		Point const * GetMidPoint(int index) const { return triple[index].mid_point; }
		Point * GetMidPoint(int index) { return triple[index].mid_point; }
		
		Node const * GetCousin(int index) const { return triple[index].cousin; }
		Node * GetCousin(int index) { return triple[index].cousin; }
		void SetCousin(int index, Node & cousin);
		
		int GetChildSeed(int child_index) const;
		void GetChildCorners(int child_index, Point * child_corners[3]) const;
		void GetChildNeighbours(int child_index, Node * child_neighbours[3]) const;
		
#if defined(VERIFY)
		void Verify() const;
#endif
		DUMP_OPERATOR_FRIEND_DECLARATION(Node);
		
		OVERLOAD_NEW_DELETE(128);
		
	private:
		flag_type flags_and_children;	//  4	/	8
		Node * parent;			//  4	/	8
	public:
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

		// The score parameters - variables which affect the node's score.
		Vector3 center;			// 12	/	12
		float area;				//  4	/	 4
		Vector3 normal;			// 12	/	12
		
		float score;			//  4	/	 4 32
	};	// 80	/
	
}

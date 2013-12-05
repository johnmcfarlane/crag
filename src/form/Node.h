//
//  Node.h
//  crag
//
//  Created by john on 5/9/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "form/Point.h"

#include "core/pointer_union.h"

namespace form
{
	class Node;
	class PointBuffer;
	class Polyhedron;
	
	
	// This is easily one of the most important classes in the formation system. 
	// You can think of it as a triangle with corners represented by the three corner instances.
	
	// This triangle is sub-divided by adding mid-points along its three edges. 
	// These are the three min_point instances.
	// Where all three mid-points are present, the Node can have four children. 
	// Think of the four triangles of the Zelda Triforce,
	// the forth (children[3]) being the upside-down center triangle.
	
	class Node
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Node();
		~Node();

		// Ensures all three mid-points are allocated and calculated.
		bool InitMidPoints(Polyhedron & polyhedron, PointBuffer & point_buffer);
		static bool InitChildCorners(Node const & parent, Node * children);

		// Calls InitMidPoints and recalcs the center position.
		void Reinit(Polyhedron & polyhedron, PointBuffer & point_buffer);
		
		bool InitScoreParameters();
		
		bool HasChildren() const { return GetChildren() != nullptr; }
		bool IsRecyclable() const { return ! HasChildren(); }
		bool IsInUse() const { return _owner != nullptr; }
		bool IsLeaf() const { return ! HasChildren(); }
		
		bool HasAllCousins() const
		{
			// Make sure all three cousins are available.
			//return triple[0].cousin != nullptr && triple[1].cousin != nullptr && triple[2].cousin != nullptr;
			return (size_t(triple[0].cousin) * size_t(triple[1].cousin) * size_t(triple[2].cousin)) != 0;
		}

		bool IsExpandable() const
		{
			ASSERT(score > 0);
			return ! HasChildren() && HasAllCousins() /*&& score != 0*/;
		}
		
		Node * GetChildren()
		{
			return _children;
		}
		Node const * GetChildren() const { return _children; }
		void SetChildren(Node * c);
		
		Node * GetParent() { return _owner.find<Node>(); }
		Node const * GetParent() const { return _owner.find<Node>(); }
		void SetParent(Node * p);
		
		Polyhedron * GetPolyhedron() { return _owner.find<Polyhedron>(); }
		Polyhedron const * GetPolyhedron() const { return _owner.find<Polyhedron>(); }
		void SetPolyhedron(Polyhedron * p);
		
		Point const * GetCorner(int index) const { return triple[index].corner; }
		Point * GetCorner(int index) { return triple[index].corner; }
		
		Point const * GetMidPoint(int index) const { return triple[index].mid_point; }
		Point * GetMidPoint(int index) { return triple[index].mid_point; }
		
		Node const * GetCousin(int index) const { return triple[index].cousin; }
		Node * GetCousin(int index) { return triple[index].cousin; }
		void SetCousin(int index, Node & cousin);
		
		int GetChildSeed(int child_index) const;
		void GetChildCorners(int child_index, Point * child_corners[3]) const;
		void GetChildNeighbours(int child_index, Node * child_neighbours[3]) const;
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Node);

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		Node * _children;

		typedef ::crag::core::pointer_union_base<sizeof(double *), alignof(double *), sizeof(double), alignof(double), Node, Polyhedron> PointerUnion;
		PointerUnion _owner;	//  4	/	8
		static_assert(sizeof(_owner) == sizeof(void*), "");
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
	};	// 80	/	128
}

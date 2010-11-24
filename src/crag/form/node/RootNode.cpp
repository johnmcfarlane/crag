/*
 *  RootNode.cpp
 *  Crag
 *
 *  Created by John on 3/14/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "RootNode.h"

#include "PointBuffer.h"

#include "geom/VectorOps.h"
#include "geom/Vector3.h"


////////////////////////////////////////////////////////////////////////////////
// Node accessor

form::RootNode & form::GetRootNode(Node & node)
{
	//VerifyArrayElement(& node, nodes, nodes_available_end);
	
	Node * iterator = & node;
	while (true) {
		Node * parent = iterator->GetParent();
		if (parent == nullptr) {
			return * reinterpret_cast<RootNode *> (iterator);
		}
		iterator = parent;
	}
}


////////////////////////////////////////////////////////////////////////////////
// RootNode

form::RootNode::RootNode()
	: owner(nullptr)
{
}

form::RootNode::RootNode(Polyhedron & _owner)
	: owner(& _owner)
{
}

form::RootNode::RootNode(RootNode const & rhs)
	: Node(rhs)
	, owner(rhs.owner)
{
}

#if DUMP
DUMP_OPERATOR_DEFINITION(form, RootNode)
{
	lhs << dynamic_cast<Node const &> (rhs);
	return lhs;
}
#endif

void form::RootNode::Init(int init_seed, Point * root_points[4])
{
	Point * corner = root_points[0];

	triple[0].corner = corner;
	triple[1].corner = corner;
	triple[2].corner = corner;
		
	center = Vector3f::Zero();
	area = 0;
	normal = Vector3f::Zero();
	score = std::numeric_limits<float>::max();
	
	//Assert(sizeof(* this) == 44 * 4);
	Assert(GetParent() == nullptr);
	seed = init_seed;
	
	for (int i = 0; i < 3; ++ i)
	{
		Triplet & t = triple[i];
		t.mid_point = root_points[i + 1];
		t.cousin = this;
	}
	
	// At this point, the four verts still need setting up.
}

void form::RootNode::GetPoints(Point * points[4])
{
	// TODO: Verify would include check that {points[0] == (triple[1]} == triple[2])
	points[0] = triple[0].corner;
	points[1] = triple[0].mid_point;
	points[2] = triple[1].mid_point;
	points[3] = triple[2].mid_point;
}

void form::RootNode::Deinit(PointBuffer & points)
{
	Assert(! HasChildren());
	
	points.Free(triple[0].corner);
	
	for (int i = 0; i < 3; ++ i) {
		triple[i].corner = nullptr;
		
		points.Free(triple[i].mid_point);
		triple[i].mid_point = nullptr;
		
		triple[i].cousin = nullptr;
	}
}

form::Polyhedron & form::RootNode::GetOwner() const
{
	return ref(owner);
}

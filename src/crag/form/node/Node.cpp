/*
 *  form/Node.cpp
 *  Crag
 *
 *  Created by john on 5/9/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "form/node/Node.h"

#include "core/Random.h"
#include "geom/VectorOps.h"


////////////////////////////////////////////////////////////////////////////////
// Triplet

form::Node::Triplet::Triplet()
: corner (nullptr)
, mid_point (nullptr)
, cousin (nullptr)
{
}

form::Node::Triplet::~Triplet()
{
	//Assert(corner == nullptr);	// fails on account of the nodes scoped in NodeBuffer::ExpandNode
	Assert(mid_point == nullptr);
	Assert(cousin == nullptr);
}


////////////////////////////////////////////////////////////////////////////////
// Node

form::Node::Node()
: children (nullptr)
, parent (nullptr)
, seed (0)
, score (0)
{
}

form::Node::~Node()
{
	Assert(children == nullptr);
	Assert(parent == nullptr);
}

bool form::Node::InitGeometry()
{
	Vector3f const & a = ref(triple[0].corner).pos;
	Vector3f const & b = ref(triple[1].corner).pos;
	Vector3f const & c = ref(triple[2].corner).pos;
	
	normal = TriangleNormal(a, b, c);
	if (! FastSafeNormalize(normal)) 
	{
		return false;
	}
	
	area = TriangleArea<float>(a, b, c);
	if (area == 0) 
	{
		return false;
	}
	
	center = (a + b + c) / 3.f;
	return true;
}

void form::Node::SetCousin(int index, Node & cousin)
{
	Triplet & this_triplet = triple[index];
	Triplet & that_triplet = cousin.triple[index];

	Assert((this_triplet.cousin == nullptr && that_triplet.cousin == nullptr) || (this_triplet.cousin->triple[index].cousin == this));
	Assert(this_triplet.mid_point == nullptr);
	
	this_triplet.cousin = & cousin;
	that_triplet.cousin = this;
	this_triplet.mid_point = that_triplet.mid_point;
}

int form::Node::GetChildSeed(int child_index) const
{
	return Random(Random(seed).GetInt() + child_index).GetInt();
}

void form::Node::GetChildCorners(int child_index, Point * child_corners[3]) const
{
	if (child_index < 3)
	{
		// Calc child's triple.
		for (int index_offset = 1; index_offset <= 2; ++ index_offset)
		{
			Point * & child_corner = child_corners[TriMod(child_index + index_offset)];
			child_corner = triple[TriMod(child_index + (3 - index_offset))].mid_point;
		}
		
		child_corners[child_index] = triple[child_index].corner;
	}
	else
	{
		child_corners[0] = triple[0].mid_point;
		child_corners[1] = triple[1].mid_point;
		child_corners[2] = triple[2].mid_point;
	}
	
	Assert(child_corners[0] != nullptr);
	Assert(child_corners[1] != nullptr);
	Assert(child_corners[2] != nullptr);
}

#if VERIFY
// Not valid for root_node!
void form::Node::Verify() const
{
	VerifyTrue(sizeof(* this) == 80 || sizeof(* this) == 128);

	if (parent != nullptr) {
		if (children != nullptr) {
			VerifyRef(* children);
			VerifyTrue(children[0].parent == this);
			VerifyTrue(children[1].parent == this);
			VerifyTrue(children[2].parent == this);
			VerifyTrue(children[3].parent == this);
		}
		
		VerifyTrue(this >= & parent->children[0] && this < & parent->children[4]);
		
		for (int i = 0; i < 3; ++ i) {
			Triplet const & t = triple[i];
			VerifyTrue(t.corner != nullptr);
			
			Node const * cousin = t.cousin;
			if (cousin != nullptr) {
				Triplet const & ct = cousin->triple[i];
				VerifyTrue(t.mid_point == ct.mid_point);
				VerifyTrue(ct.cousin == this);
			}
		}
		
		VerifyTrue(area > 0);
		VerifyEqual(LengthSq(normal), 1, 0.001f);
		VerifyTrue(score >= 0);
	}
	else {
		VerifyTrue(children == nullptr);
		
		for (int i = 0; i < 3; ++ i) {
			Triplet const & t = triple[i];
			VerifyTrue(t.corner == nullptr);
			VerifyTrue(t.mid_point == nullptr);
			VerifyTrue(t.cousin == nullptr);
		}
		
		VerifyTrue(score == 0);
	}

}
#endif

#if DUMP
DUMP_OPERATOR_DEFINITION(form, Node)
{
	lhs << lhs.NewLine() << "children:" << rhs.children;
	if (rhs.children != nullptr) {
		lhs << "(" << ',' << (rhs.children + 1) << ',' << (rhs.children + 2) << ',' << (rhs.children + 3) << ')';
	}
	lhs << "; parent:" << rhs.parent;
	lhs << "; seed:" << rhs.seed << ';';
	
	DumpStream indented(lhs);
	for (int i = 0; i < 3; ++ i) {
		indented << indented.NewLine() << "triplet[" << i << "]:(";
		indented << "corner:" << rhs.triple[i].corner;
		indented << "; mid_point:" << rhs.triple[i].mid_point;
		indented << "; cousin:" << rhs.triple[i].cousin;
		indented << ");";
	}
	
	lhs << lhs.NewLine() << "center:" << rhs.center;
	lhs << ";area:" << rhs.area;
	lhs << ";normal:" << rhs.normal;
	lhs << ";score:" << rhs.score;
	
	return lhs;
}
#endif

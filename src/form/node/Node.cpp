//
//  Node.cpp
//  crag
//
//  Created by john on 5/9/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "form/node/Node.h"
#include "form/node/PointBuffer.h"
#include "form/node/Shader.h"

#include "form/Formation.h"

#include "form/scene/Polyhedron.h"

#include "core/Random.h"

#include "geom/Intersection.h"


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
	//ASSERT(corner == nullptr);	// fails on account of the nodes scoped in NodeBuffer::ExpandNode
	ASSERT(mid_point == nullptr);
	ASSERT(cousin == nullptr);
}


////////////////////////////////////////////////////////////////////////////////
// Node

form::Node::Node()
: flags_and_children (0)
, _parent (nullptr)
, seed (0)
, score (0)
{
	VerifyObject(* this);
}

form::Node::~Node()
{
	ASSERT(GetChildren() == nullptr);
	ASSERT(_parent == nullptr);
}

// Makes sure node's three mid-points are non-null or returns false.
bool form::Node::InitMidPoints(Polyhedron & polyhedron, PointBuffer & point_buffer)
{
	// success
	bool success = true;
	
	Shader const & shader = polyhedron.GetFormation().GetShader();
	
	// Make sure all mid-points exist.
	for (int triplet_index = 0; triplet_index < 3; ++ triplet_index)
	{
		// If the mid-point does not already exist,
		Node::Triplet & t = triple[triplet_index];
		if (t.mid_point == nullptr)
		{
			// This function shouldn't be called unless all three cousins exist.
			Node & cousin = ref(t.cousin);

			// Create the mid-point.
			Point * new_point = point_buffer.Create();
			if (new_point == nullptr)
			{
				// We should always have enough points in the buffer.
				ASSERT(false);
				
				success = false;
				continue;
			}

			// Try and set its value.
			if (! shader.InitMidPoint(polyhedron, ref(this), ref(t.cousin), triplet_index, ref(new_point)))
			{
				// If this failed, free it up and return.
				
				// Note that there may be one or two other mid-points that were created.
				// That's ok. They're valid and they might make it easier to expand this
				// or neighboring nodes in the future. This is because sometimes, a
				// cousin is destroyed. This makes it impossible to create the point, but
				// the point is still valid. 
				point_buffer.Destroy(new_point);
				
				// Because of this, we try and create all three mid-points - even after we
				// know that failure is inevitable. 
				success = false;
				continue;
			}

			// The mid-point was successfully allocated and initialized.
			
			// Assign the mid-point to this node and the cousin.
			t.mid_point = cousin.triple[triplet_index].mid_point = new_point;
		}
	}
	
	return success;
}

// This step in the initialization of a node is performed 
// before it is determined that the node can be expanded.
bool form::Node::InitChildCorners(Node const & parent, Node * children)
{
	ASSERT(parent.GetChildren() == nullptr);
	
	Node::Triplet const * parent_triple = parent.triple;
	
	Node * child = children;
	Node::Triplet * child_triple;
	
	child_triple = child->triple;
	child_triple[0].corner = parent_triple[0].corner;
	child_triple[1].corner = parent_triple[2].mid_point;
	child_triple[2].corner = parent_triple[1].mid_point;
	if (! child->InitScoreParameters())
	{
		return false;
	}
	
	++ child;
	
	child_triple = child->triple;
	child_triple[0].corner = parent_triple[2].mid_point;
	child_triple[1].corner = parent_triple[1].corner;
	child_triple[2].corner = parent_triple[0].mid_point;
	if (! child->InitScoreParameters())
	{
		return false;
	}
	
	++ child;
	
	child_triple = child->triple;
	child_triple[0].corner = parent_triple[1].mid_point;
	child_triple[1].corner = parent_triple[0].mid_point;
	child_triple[2].corner = parent_triple[2].corner;
	if (! child->InitScoreParameters())
	{
		return false;
	}
	
	++ child;
	
	child_triple = child->triple;
	child_triple[0].corner = parent_triple[0].mid_point;
	child_triple[1].corner = parent_triple[1].mid_point;
	child_triple[2].corner = parent_triple[2].mid_point;
	if (! child->InitScoreParameters())
	{
		return false;
	}
	
	ASSERT(parent.triple[0].corner == children[0].triple[0].corner);
	
	return true;
}

// This gets called when the local origin is changed.
// It recalculates all positional data, i.e. the mid-points and the center. 
void form::Node::Reinit(Polyhedron & polyhedron, PointBuffer & point_buffer)
{
	Shader const & shader = polyhedron.GetFormation().GetShader();
	
	// Make sure all mid-points exist.
	for (int triplet_index = 0; triplet_index < 3; ++ triplet_index)
	{
		Node::Triplet & t = triple[triplet_index];
		if (t.cousin != nullptr)
		{
			if (t.mid_point != nullptr)
			{
				Node & cousin = ref(t.cousin);
				if (& cousin > this)
				{
					Point & mid_point = ref(t.mid_point);
					shader.InitMidPoint(polyhedron, * this, cousin, triplet_index, mid_point);
				}
			}
		}
		else 
		{
			if (t.mid_point != nullptr)
			{
				// There's a mid-point but the cousin used to calculate it has since been destroyed.
				// There's no easy way to calculate the new position (except maybe delta it).
				// So just remove it instead.
				point_buffer.Destroy(t.mid_point);
				t.mid_point = nullptr;
				
				ASSERT(GetChildren() == nullptr);
			}
		}
	}
	
	geom::Vector3f const & a = ref(triple[0].corner).pos;
	geom::Vector3f const & b = ref(triple[1].corner).pos;
	geom::Vector3f const & c = ref(triple[2].corner).pos;
	center = (a + b + c) / 3.f;
}

bool form::Node::InitScoreParameters()
{
	geom::Vector3f const & a = ref(triple[0].corner).pos;
	geom::Vector3f const & b = ref(triple[1].corner).pos;
	geom::Vector3f const & c = ref(triple[2].corner).pos;
	
	normal = TriangleNormal(a, b, c);
	ASSERT(normal == normal);
	ASSERT(LengthSq(normal) > 0);
	FastNormalize(normal);
	
	area = geom::TriangleArea<float>(a, b, c);
	if (area == 0) 
	{
		return false;
	}
	
	center = (a + b + c) / 3.f;
	return true;
}

void form::Node::SetFlags(flag_type f) 
{
	ASSERT ((f & pointer_mask) == 0);
	flags_and_children &= pointer_mask;
	flags_and_children |= f; 
}

void form::Node::SetChildren(Node * c) 
{ 
	ASSERT ((reinterpret_cast<flag_type>(c) & flag_mask) == 0);
	flags_and_children &= flag_mask;
	flags_and_children |= reinterpret_cast<flag_type>(c); 
}

void form::Node::SetParent(Node * p) 
{ 
	_parent = p; 
}

void form::Node::SetCousin(int index, Node & cousin)
{
	Triplet & this_triplet = triple[index];
	Triplet & that_triplet = cousin.triple[index];

	ASSERT((this_triplet.cousin == nullptr && that_triplet.cousin == nullptr) || (this_triplet.cousin->triple[index].cousin == this));
	ASSERT(this_triplet.mid_point == nullptr);
	
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
	
	ASSERT(child_corners[0] != nullptr);
	ASSERT(child_corners[1] != nullptr);
	ASSERT(child_corners[2] != nullptr);
}

#if defined(VERIFY)
// Not valid for root_node!
void form::Node::Verify() const
{
	VerifyTrue(sizeof(* this) == 80 || sizeof(* this) == 128);
	VerifyEqual(reinterpret_cast<flag_type>(this) & flag_mask, static_cast<flag_type>(0));

	if (_parent != nullptr) 
	{
		int child_index = (this - _parent->GetChildren());
		VerifyTrue(_parent->GetChildren() + child_index == this);
		VerifyTrue(child_index >= 0 && child_index < 4);
		VerifyTrue(seed == _parent->GetChildSeed(child_index));
		
		for (int i = 0; i < 3; ++ i) 
		{
			Triplet const & t = triple[i];
			VerifyTrue(t.corner != nullptr);
			
			Node const * cousin = t.cousin;
			if (cousin != nullptr) 
			{
				Triplet const & ct = cousin->triple[i];
				VerifyTrue(t.mid_point == ct.mid_point);
				VerifyTrue(ct.cousin == this);
			}
		}
		
		VerifyTrue(area > 0);
		VerifyNearlyEqual(LengthSq(normal), 1.f, .001f);
		VerifyTrue(score >= 0);
	}
	else {
		VerifyTrue(GetChildren() == nullptr);
		
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

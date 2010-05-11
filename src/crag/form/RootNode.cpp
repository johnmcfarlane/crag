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

#include "VertexBuffer.h"

#include "core/VectorOps.h"
#include "core/Vector3.h"


////////////////////////////////////////////////////////////////////////////////
// RootNode

form::RootNode::RootNode(Model & _owner)
: owner(_owner)
{
}

#if DUMP
DUMP_OPERATOR_DEFINITION(form, RootNode)
{
	lhs << dynamic_cast<Node const &> (rhs);
	return lhs;
}
#endif

void form::RootNode::Init(int init_seed, VertexBuffer & vertices)
{
	Vertex * corner = vertices.Alloc();

	triple[0].corner = corner;
	triple[1].corner = corner;
	triple[2].corner = corner;
		
	center = Vector3f::Zero();
	area = 0;
	normal = Vector3f::Zero();
	score = std::numeric_limits<float>::max();
	
	//Assert(sizeof(* this) == 44 * 4);
	Assert(parent == nullptr);
	seed = init_seed;
	
	for (int i = 0; i < 3; ++ i)
	{
		Triplet & t = triple[i];
		t.mid_point = vertices.Alloc();
		t.cousin = this;
	}
	
	// At this point, the four verts still need setting up.
}

void form::RootNode::Deinit(VertexBuffer & vertices)
{
	vertices.Free(triple[0].corner);
	
	for (int i = 0; i < 3; ++ i) {
		triple[i].corner = nullptr;
		
		vertices.Free(triple[i].mid_point);
		triple[i].mid_point = nullptr;
		
		triple[i].cousin = nullptr;
	}
}

form::Model & form::RootNode::GetOwner() const
{
	return owner;
}

void form::RootNode::SetCenter(Vector3d const & center, double scale)
{
	Assert(children == nullptr);
	
	SetVertexCenter(ref(triple[0].corner), Vector3d(-1, -1, -1), center, scale);
	SetVertexCenter(ref(triple[0].mid_point), Vector3d(-1,  1,  1), center, scale);
	SetVertexCenter(ref(triple[1].mid_point), Vector3d(1,  -1,  1), center, scale);
	SetVertexCenter(ref(triple[2].mid_point), Vector3d(1,  1,  -1), center, scale);
}

void form::RootNode::SetVertexCenter(Vertex & vert, Vector3d const & relative_pos, Vector3d const & center, double scale)
{
	Vector3d dir = Normalized(relative_pos);
	vert.pos = dir * scale + center;
	vert.red = 255;
	vert.green = 255;
	vert.blue = 255;
	vert.norm = dir;
}


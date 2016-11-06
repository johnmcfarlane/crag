//
//  NodeBuffer.cpp
//  crag
//
//  Created by John on 2/17/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Node.h"
#include "NodeBuffer.h"
#include "Point.h"

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// form::NodeBuffer member definitions

#if defined(CRAG_VERIFY_ENABLED)
CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(NodeBuffer, object)
	CRAG_VERIFY_ARRAY_POINTER(object._nodes_used_end, object._nodes, object._nodes_end);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void NodeBuffer::VerifyUsed(Node const & node) const
{
	CRAG_VERIFY_ARRAY_ELEMENT(& node, _nodes, _nodes_used_end);

	CRAG_VERIFY(node);
	
	auto parent = node.GetParent();
	CRAG_VERIFY_TRUE(parent);
	
	auto child_index = & node - parent->GetChildren();
	CRAG_VERIFY_OP(child_index, >=, 0);
	CRAG_VERIFY_OP(child_index, <, 4);

	auto polyhedron = node.GetPolyhedron();
	CRAG_VERIFY_FALSE(polyhedron);

	for (int i = 0; i < 3; ++ i)
	{
		Node::Triplet const & t = node.triple[i];
		CRAG_VERIFY_TRUE(t.corner);
	}

	CRAG_VERIFY_OP(node.score, >, 0);
}

void NodeBuffer::VerifyUnused(Node const & node) const
{
	CRAG_VERIFY_ARRAY_ELEMENT(& node, _nodes_used_end, _nodes_end);

	CRAG_VERIFY(node);
	
	CRAG_VERIFY_FALSE(node.GetParent());
	CRAG_VERIFY_FALSE(node.GetPolyhedron());

	CRAG_VERIFY_FALSE(node.GetParent());
	CRAG_VERIFY_FALSE(node.HasChildren());
	CRAG_VERIFY_EQUAL(node.score, 0);

	for (int i = 0; i < 3; ++ i)
	{
		Node::Triplet const & t = node.triple[i];
		CRAG_VERIFY_FALSE(t.corner);
		CRAG_VERIFY_FALSE(t.mid_point);
		CRAG_VERIFY_FALSE(t.cousin);
	}
}
#endif

NodeBuffer::NodeBuffer(int max_num_nodes)
: _nodes(reinterpret_cast<Node *>(Allocate(static_cast<int>(sizeof(Node)) * max_num_nodes, 128)))
, _nodes_used_end(_nodes)
, _nodes_end(_nodes + max_num_nodes)
{
	ZeroArray(_nodes, max_num_nodes);

	CRAG_VERIFY(* this);
}

NodeBuffer::~NodeBuffer()
{
	CRAG_VERIFY(* this);

	//delete nodes;
	Free(_nodes);
}

void NodeBuffer::Clear()
{
	_nodes_used_end = _nodes;
}

void NodeBuffer::Push(int num_nodes)
{
	auto new_nodes_used_end = _nodes_used_end + num_nodes;
	CRAG_VERIFY_ARRAY_POINTER(new_nodes_used_end, _nodes, _nodes_end);
	
	_nodes_used_end = new_nodes_used_end;
}

void NodeBuffer::Pop(int num_nodes)
{
	auto new_nodes_used_end = _nodes_used_end - num_nodes;
	CRAG_VERIFY_ARRAY_POINTER(new_nodes_used_end, _nodes, _nodes_end);
	
	_nodes_used_end = new_nodes_used_end;
}

void NodeBuffer::ResetNodeOrigins(Vector3 const & origin_delta)
{
	for (Node * node = _nodes; node != _nodes_used_end; ++ node)
	{
		Node::Triplet * triple = node->triple;
		for (int i = 0; i < 3; ++ i)
		{
			Node::Triplet & t = triple[i];
			if (t.mid_point != nullptr)
			{
				// We only want to increment each mid-point once, 
				// but it's often pointed to by two nodes.
				// This is an arbitrary test to ensure it's only incremented for one node.
				// Note that if cousin is null, the condition still passes. 
				if (node > t.cousin)
				{
					t.mid_point->pos -= origin_delta;
				}
			}
		}
		
		node->center += origin_delta;
	}
}

bool NodeBuffer::IsEmpty() const
{
	return _nodes_used_end == _nodes;
}

int NodeBuffer::GetSize() const
{
	return core::get_index(_nodes, * _nodes_used_end);
}

int NodeBuffer::GetCapacity() const
{
	return core::get_index(_nodes, * _nodes_end);
}

Node const & NodeBuffer::operator [] (int index) const
{
	CRAG_VERIFY_OP(index, <, GetSize());
	return _nodes[index];
}

Node const * NodeBuffer::begin() const
{
	return _nodes;
}

Node const * NodeBuffer::end() const
{
	return _nodes_used_end;
}

Node * NodeBuffer::begin()
{
	return _nodes;
}

Node * NodeBuffer::end()
{
	return _nodes_used_end;
}


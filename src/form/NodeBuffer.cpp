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

using namespace form;

#if defined(VERIFY)
void NodeBuffer::Verify() const
{
	VerifyArrayPointer(_nodes_used_end, _nodes, _nodes_end);
}

void NodeBuffer::VerifyUsed(Node const & n) const
{
	VerifyRef(n);
	VerifyArrayElement(&n, _nodes, _nodes_used_end);
}
#endif

NodeBuffer::NodeBuffer(size_t max_num_nodes)
: _nodes(reinterpret_cast<Node *>(Allocate(sizeof(Node) * max_num_nodes, 128)))
, _nodes_used_end(_nodes)
, _nodes_end(_nodes + max_num_nodes)
{
	ZeroArray(_nodes, max_num_nodes);

	VerifyObject(* this);
}

NodeBuffer::~NodeBuffer()
{
	VerifyObject(* this);

	//delete nodes;
	Free(_nodes);
}

void NodeBuffer::Clear()
{
	_nodes_used_end = _nodes;
}

void NodeBuffer::Push(std::size_t num_nodes)
{
	auto new_nodes_used_end = _nodes_used_end + num_nodes;
	VerifyArrayPointer(new_nodes_used_end, _nodes, _nodes_end);
	
	_nodes_used_end = new_nodes_used_end;
}

void NodeBuffer::Pop(std::size_t num_nodes)
{
	auto new_nodes_used_end = _nodes_used_end - num_nodes;
	VerifyArrayPointer(new_nodes_used_end, _nodes, _nodes_end);
	
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

std::size_t NodeBuffer::GetSize() const
{
	return _nodes_used_end - _nodes;
}

std::size_t NodeBuffer::GetCapacity() const
{
	return _nodes_end - _nodes;
}

Node const & NodeBuffer::operator [] (std::size_t index) const
{
	VerifyOp(index, <, GetSize());
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


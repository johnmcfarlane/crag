//
//  gfx/object/Object.cpp
//  crag
//
//  Created by John McFarlane on 7/23/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Object.h"

#include "BranchNode.h"
#include "LeafNode.h"


using namespace gfx;


Object::Object(Layer::Map::type layers, NodeType node_type)
: _parent(nullptr)
, _layers(layers)
, _node_type(node_type)
{ 
}

Object::~Object() 
{
	if (_parent != nullptr)
	{
		_parent->RemoveChild(* this);
	}
}

#if defined(VERIFY)
void Object::Verify() const
{
	ChildList::verify(* this);
	
	Layer::Verify(_layers);
	VerifyTrue(_node_type == branch || _node_type == leaf);
}
#endif

void Object::Init()
{
}

void Object::Deinit()
{
}

bool Object::IsInLayer(Layer::type layer) const
{
	Layer::Verify(layer);
	
	return (_layers & Layer::ToMap(layer)) != 0;
}

bool Object::IsInLayers(Layer::Map::type layers) const
{
	Layer::Verify(layers);
	
	return (_layers & layers) != 0;
}

// typically called by derived class
void Object::AddToLayer(Layer::type layer)
{
	Layer::Verify(layer);
	
	_layers |= layer;
	
	if (_parent != nullptr)
	{
		_parent->AddToLayer(layer);
	}
}

Object::NodeType Object::GetNodeType() const
{
	return _node_type;
}

LeafNode & Object::GetLeafNodeRef()
{
	Assert(GetNodeType() == leaf);
	return static_cast<LeafNode &>(* this);
}

LeafNode const & Object::GetLeafNodeRef() const
{
	Assert(GetNodeType() == leaf);
	return static_cast<LeafNode const &>(* this);
}

LeafNode * Object::GetLeafNodePtr()
{
	return (GetNodeType() == leaf)
	? static_cast<LeafNode *>(this)
	: nullptr;
}

LeafNode const * Object::GetLeafNodePtr() const
{
	return (GetNodeType() == leaf)
	? static_cast<LeafNode const *>(this)
	: nullptr;
}

BranchNode & Object::GetBranchNodeRef()
{
	Assert(GetNodeType() == branch);
	return static_cast<BranchNode &>(* this);
}

BranchNode const & Object::GetBranchNodeRef() const
{
	Assert(GetNodeType() == branch);
	return static_cast<BranchNode const &>(* this);
}

BranchNode * Object::GetBranchNodePtr()
{
	return (GetNodeType() == branch)
	? static_cast<BranchNode *>(this)
	: nullptr;
}

BranchNode const * Object::GetBranchNodePtr() const
{
	return (GetNodeType() == branch)
	? static_cast<BranchNode const *>(this)
	: nullptr;
}

BranchNode * Object::GetParent()
{
	return _parent;
}

BranchNode const * Object::GetParent() const
{
	return _parent;
}

void Object::SetParent(BranchNode * parent)
{
	_parent = parent;
}

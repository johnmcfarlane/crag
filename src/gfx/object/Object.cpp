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


Object::Object(NodeType node_type, Layer::Map::type layers)
: _parent(nullptr)
, _uid(Uid::Create())
, _node_type(node_type)
, _layers(layers)
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

void Object::Init(Scene const & scene)
{
}

void Object::Deinit()
{
}

Uid Object::GetUid() const
{
	return _uid;
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

LeafNode & Object::CastLeafNodeRef()
{
	Assert(GetNodeType() == leaf);
	return static_cast<LeafNode &>(* this);
}

LeafNode const & Object::CastLeafNodeRef() const
{
	Assert(GetNodeType() == leaf);
	return static_cast<LeafNode const &>(* this);
}

LeafNode * Object::CastLeafNodePtr()
{
	return (GetNodeType() == leaf)
	? static_cast<LeafNode *>(this)
	: nullptr;
}

LeafNode const * Object::CastLeafNodePtr() const
{
	return (GetNodeType() == leaf)
	? static_cast<LeafNode const *>(this)
	: nullptr;
}

BranchNode & Object::CastBranchNodeRef()
{
	Assert(GetNodeType() == branch);
	return static_cast<BranchNode &>(* this);
}

BranchNode const & Object::CastBranchNodeRef() const
{
	Assert(GetNodeType() == branch);
	return static_cast<BranchNode const &>(* this);
}

BranchNode * Object::CastBranchNodePtr()
{
	return (GetNodeType() == branch)
	? static_cast<BranchNode *>(this)
	: nullptr;
}

BranchNode const * Object::CastBranchNodePtr() const
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

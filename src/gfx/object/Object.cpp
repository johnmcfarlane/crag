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


Object::Object(NodeType node_type)
: _parent(nullptr)
, _uid(Uid::Create())
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
	
	VerifyTrue(_node_type == branch || _node_type == leaf);
}
#endif

bool Object::Init(Renderer & renderer)
{
	return true;
}

void Object::Deinit(Scene & scene)
{
}

gfx::Transformation const & Object::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	return model_view;
}

Uid Object::GetUid() const
{
	return _uid;
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

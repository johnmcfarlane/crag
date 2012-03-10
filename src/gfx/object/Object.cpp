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
, _node_type(node_type)
{ 
}

Object::~Object() 
{
	if (_parent != nullptr)
	{
		OrphanChild(* this);
	}
}

#if defined(VERIFY)
void Object::Verify() const
{
	super::Verify();
	
	List::verify(* this);
	
	VerifyPtr(_parent);
	VerifyTrue(_node_type == branch || _node_type == leaf);
	if (_parent != nullptr)
	{
		VerifyTrue(_parent->IsChild(* this));
	}
}
#endif

void Object::Deinit(Scene & scene)
{
}

gfx::Transformation const & Object::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	return model_view;
}

Object::NodeType Object::GetNodeType() const
{
	return _node_type;
}

LeafNode & Object::CastLeafNodeRef()
{
	ASSERT(GetNodeType() == leaf);
	return static_cast<LeafNode &>(* this);
}

LeafNode const & Object::CastLeafNodeRef() const
{
	ASSERT(GetNodeType() == leaf);
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
	ASSERT(GetNodeType() == branch);
	return static_cast<BranchNode &>(* this);
}

BranchNode const & Object::CastBranchNodeRef() const
{
	ASSERT(GetNodeType() == branch);
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

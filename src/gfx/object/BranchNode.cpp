//
//  BranchNode.cpp
//  crag
//
//  Created by John McFarlane on 2011/11/21.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "BranchNode.h"


using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::ListObejct definitions


BranchNode::BranchNode()
: Object(branch, Layer::Map::none)
{
}

BranchNode::~BranchNode()
{
	Assert(IsEmpty());
}

#if defined(VERIFY)
void BranchNode::Verify() const
{
	super::Verify();
	
	_children.verify();
}
#endif

bool BranchNode::IsEmpty() const
{
	return _children.empty();
}

void BranchNode::AddChild(Object & child)
{
	Assert(child.GetParent() == nullptr);
	child.SetParent(this);
	
	_children.push_back(child);
}

void BranchNode::RemoveChild(Object & child)
{
	_children.remove(child);
}

BranchNode::ChildList::iterator BranchNode::Begin()
{
	return _children.begin();
}

BranchNode::ChildList::const_iterator BranchNode::Begin() const
{
	return _children.begin();
}

BranchNode::ChildList::iterator BranchNode::End()
{
	return _children.end();
}

BranchNode::ChildList::const_iterator BranchNode::End() const
{
	return _children.end();
}

BranchNode * BranchNode::CastListObject()
{
	return this;
}

BranchNode const * BranchNode::CastListObject() const
{
	return this;
}

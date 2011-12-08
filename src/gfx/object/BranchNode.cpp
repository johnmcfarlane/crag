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
	Layer::Map::type child_map = child.GetLayers();
	for (BranchNode * ancestor = this; ancestor != nullptr; ancestor = ancestor->GetParent())
	{
		ancestor->AddToLayers(child_map);
	}	
	
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

gfx::Transformation const & BranchNode::GetTransformation() const
{
	return _transformation;
}

void BranchNode::SetTransformation(Transformation const & transformation)
{
	_transformation = transformation;
}

void BranchNode::GetAccumulatedTransformation(Transformation & transformation) const
{
	BranchNode const * ancestor = this;
	do
	{
		Transformation const & ancestor_transformation = ancestor->GetTransformation();
		transformation = ancestor_transformation * transformation;
		
		ancestor = ancestor->GetParent();
	}	while (ancestor != nullptr);
}

void BranchNode::Update(UpdateParams const & params)
{
	_transformation = params.transformation;
}

BranchNode * BranchNode::CastListObject()
{
	return this;
}

BranchNode const * BranchNode::CastListObject() const
{
	return this;
}

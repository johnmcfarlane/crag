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
// gfx::BranchNode definitions

DEFINE_POOL_ALLOCATOR(BranchNode, 128);

BranchNode::BranchNode(super::Init const & init, Transformation const & transformation)
: Object(init, branch)
, _transformation(transformation)
{
}

BranchNode::~BranchNode()
{
	ASSERT(IsEmpty());
}

#if defined(VERIFY)
void BranchNode::Verify() const
{
	super::Verify();
	
	_children.verify();
	
	for (List::const_iterator i = _children.begin(), end = _children.begin(); i != end; ++ i)
	{
		Object const & child = static_cast<Object const &>(* i);
		VerifyTrue(child.GetParent() == this);
	}
	
	VerifyObject(_transformation);
}
#endif

bool BranchNode::IsEmpty() const
{
	return _children.empty();
}

bool BranchNode::IsChild(Object const & child) const
{
	return _children.contains(child);
}

void gfx::AdoptChild(Object & child, BranchNode & parent)
{
	ASSERT(child._parent == nullptr);
	child._parent = & parent;
	
	ASSERT(! parent._children.contains(child));
	parent._children.push_back(child);
}

void gfx::OrphanChild(Object & child, BranchNode & parent)
{
	ASSERT(child._parent == & parent);
	child._parent = nullptr;
	
	ASSERT(parent._children.contains(child));
	parent._children.remove(child);
}

void gfx::OrphanChild(Object & child)
{
	BranchNode * parent = child._parent;
	if (parent != nullptr)
	{
		OrphanChild(child, * parent);
	}
}

BranchNode::List::iterator BranchNode::Begin()
{
	return _children.begin();
}

BranchNode::List::const_iterator BranchNode::Begin() const
{
	return _children.begin();
}

BranchNode::List::iterator BranchNode::End()
{
	return _children.end();
}

BranchNode::List::const_iterator BranchNode::End() const
{
	return _children.end();
}

Object & BranchNode::Front()
{
	return _children.front();
}

Object const & BranchNode::Front() const
{
	return _children.front();
}
		
Object & BranchNode::Back()
{
	return _children.back();
}

Object const & BranchNode::Back() const
{
	return _children.back();
}

Transformation const & BranchNode::Transform(Transformation const & model_view, Transformation & scratch) const
{
	scratch = model_view * _transformation;
	VerifyObject(scratch);
	return scratch;
}

Transformation const & BranchNode::GetTransformation() const
{
	return _transformation;
}

void BranchNode::SetTransformation(Transformation const & transformation)
{
	_transformation = transformation;
	VerifyObject(_transformation);
}

Transformation BranchNode::GetModelTransformation() const
{
	BranchNode const * ancestor = GetParent();
	
	if (ancestor == nullptr)
	{
		// This is the root node; return the identity.
		return Transformation();
	}

	Transformation model_transformation = GetTransformation();
	while (true)
	{
		BranchNode const * parent = ancestor->GetParent();
		if (parent == nullptr)
		{
			// Accumulate transformations up to - and excluding - the root node.
			return model_transformation;
		}

		Transformation const & ancestor_transformation = ancestor->GetTransformation();
		model_transformation = ancestor_transformation * model_transformation;
		
		ancestor = parent;
	}
}

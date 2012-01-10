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
: Object(branch)
, _transformation(Transformation::Matrix::Identity())
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
	Assert(! _children.contains(child));
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

gfx::Transformation const & BranchNode::Transform(Renderer & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const override
{
	return scratch = model_view * _transformation;
}

gfx::Transformation const & BranchNode::GetTransformation() const
{
	return _transformation;
}

void BranchNode::SetTransformation(gfx::Transformation const & transformation)
{
	_transformation = transformation;
}

gfx::Transformation BranchNode::GetModelTransformation() const
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

void BranchNode::Update(UpdateParams const & params, Renderer & renderer)
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

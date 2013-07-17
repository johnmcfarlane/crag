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

#include "LeafNode.h"

using namespace gfx;


Object::Object(Init const & init, Transformation const & local_transformation)
: super(init)
, _parent(nullptr)
, _local_transformation(local_transformation)
{ 
	VerifyObject(* this);
}

Object::~Object() 
{
	ASSERT(IsEmpty());

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

	if (_parent != nullptr)
	{
		VerifyTrue(IsChild(* this, * _parent));
	}

	_children.verify();
	
	for (List::const_iterator i = _children.begin(), end = _children.begin(); i != end; ++ i)
	{
		Object const & child = static_cast<Object const &>(* i);
		VerifyTrue(child.GetParent() == this);
	}
	
	VerifyObject(_local_transformation);

	auto scale = _local_transformation.GetScale();
	VerifyTrue(NearEqual<Scalar>(scale.x, 1, 0.001f));
	VerifyTrue(NearEqual<Scalar>(scale.y, 1, 0.001f));
	VerifyTrue(NearEqual<Scalar>(scale.z, 1, 0.001f));
}
#endif

LeafNode & Object::CastLeafNodeRef()
{
	ASSERT(false);
	return static_cast<LeafNode &>(* this);
}

LeafNode const & Object::CastLeafNodeRef() const
{
	ASSERT(false);
	return static_cast<LeafNode const &>(* this);
}

LeafNode * Object::CastLeafNodePtr()
{
	return nullptr;
}

LeafNode const * Object::CastLeafNodePtr() const
{
	return nullptr;
}

bool gfx::IsChild(Object const & child, Object const & parent)
{
	return parent._children.contains(child);
}

void gfx::AdoptChild(Object & child, Object & parent)
{
	ASSERT(child._parent == nullptr);
	child._parent = & parent;
	
	ASSERT(! parent._children.contains(child));
	parent._children.push_back(child);
}

void gfx::OrphanChild(Object & child, Object & parent)
{
	ASSERT(child._parent == & parent);
	child._parent = nullptr;
	
	ASSERT(parent._children.contains(child));
	parent._children.remove(child);
}

void gfx::OrphanChild(Object & child)
{
	Object * parent = child._parent;
	if (parent != nullptr)
	{
		OrphanChild(child, * parent);
	}
}

Object * Object::GetParent()
{
	return _parent;
}

Object const * Object::GetParent() const
{
	return _parent;
}

bool Object::IsEmpty() const
{
	return _children.empty();
}

Object::List::iterator Object::Begin()
{
	return _children.begin();
}

Object::List::const_iterator Object::Begin() const
{
	return _children.begin();
}

Object::List::iterator Object::End()
{
	return _children.end();
}

Object::List::const_iterator Object::End() const
{
	return _children.end();
}

Object & Object::Front()
{
	return _children.front();
}

Object const & Object::Front() const
{
	return _children.front();
}
		
Object & Object::Back()
{
	return _children.back();
}

Object const & Object::Back() const
{
	return _children.back();
}

Transformation const & Object::GetLocalTransformation() const
{
	return _local_transformation;
}

void Object::SetLocalTransformation(Transformation const & local_transformation)
{
	_local_transformation = local_transformation;
	VerifyObject(_local_transformation);
}

Transformation Object::GetModelTransformation() const
{
	Object const * ancestor = GetParent();
	
	if (ancestor == nullptr)
	{
		// This is the root node; return the identity.
		return Matrix44::Identity();
	}

	Transformation model_transformation = GetLocalTransformation();
	while (true)
	{
		Object const * parent = ancestor->GetParent();
		if (parent == nullptr)
		{
			// Accumulate transformations up to - and excluding - the root node.
			return model_transformation;
		}

		Transformation const & ancestor_transformation = ancestor->GetLocalTransformation();
		model_transformation = ancestor_transformation * model_transformation;
		
		ancestor = parent;
	}
}

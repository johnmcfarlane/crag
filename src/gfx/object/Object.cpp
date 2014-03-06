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
	CRAG_VERIFY(* this);
}

Object::~Object() 
{
	ASSERT(_children.empty());

	if (_parent != nullptr)
	{
		OrphanChild(* this);
	}
}

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

Object::List & Object::GetChildren()
{
	return _children;
}

Object::List const & Object::GetChildren() const
{
	return _children;
}

Transformation const & Object::GetLocalTransformation() const
{
	return _local_transformation;
}

void Object::SetLocalTransformation(Transformation const & local_transformation)
{
	_local_transformation = local_transformation;
	CRAG_VERIFY(_local_transformation);
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

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Object, object)
	CRAG_VERIFY(static_cast<super const &>(object));
	
	List::verify(object);
	
	CRAG_VERIFY(object._parent);

	if (object._parent != nullptr)
	{
		CRAG_VERIFY_TRUE(IsChild(object, * object._parent));
	}

	CRAG_VERIFY(object._children);
	
	for (List::const_iterator i = object._children.begin(), end = object._children.begin(); i != end; ++ i)
	{
		Object const & child = static_cast<Object const &>(* i);
		CRAG_VERIFY_TRUE(child.GetParent() == & object);
	}
	
	CRAG_VERIFY(object._local_transformation);

	auto scale = object._local_transformation.GetScale();
	CRAG_VERIFY_NEARLY_EQUAL(scale.x, 1, 0.001f);
	CRAG_VERIFY_NEARLY_EQUAL(scale.y, 1, 0.001f);
	CRAG_VERIFY_NEARLY_EQUAL(scale.z, 1, 0.001f);
CRAG_VERIFY_INVARIANTS_DEFINE_END

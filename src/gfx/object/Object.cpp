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

#include "gfx/Program.h"
#include "gfx/VboResource.h"

using namespace gfx;

////////////////////////////////////////////////////////////////////////////////
// gfx::Object helper function definitions

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

////////////////////////////////////////////////////////////////////////////////
// gfx::Object member definitions

Object::Object(Engine & engine, Transformation const & local_transformation, Layer layer, bool casts_shadow)
: super(engine)
, _parent(nullptr)
, _local_transformation(local_transformation)
, _layer(layer)
, _program(nullptr)
, _vbo_resource(nullptr)
, _casts_shadow(casts_shadow)
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
	CRAG_VERIFY(object._model_view_transformation);

	auto scale = object._local_transformation.GetScale();
	CRAG_VERIFY_NEARLY_EQUAL(scale.x, 1.f, 0.001f);
	CRAG_VERIFY_NEARLY_EQUAL(scale.y, 1.f, 0.001f);
	CRAG_VERIFY_NEARLY_EQUAL(scale.z, 1.f, 0.001f);

	CRAG_VERIFY(object._program);
	CRAG_VERIFY(object._vbo_resource);
	CRAG_VERIFY(object._casts_shadow);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void Object::UpdateModelViewTransformation(Transformation const & model_view_transformation)
{
	SetModelViewTransformation(model_view_transformation);
}

void Object::SetModelViewTransformation(Transformation const & model_view_transformation)
{
	_model_view_transformation = model_view_transformation;
	
	Transformation::Matrix44 const & matrix = _model_view_transformation.GetMatrix();
	_render_depth = matrix[1][3];

	CRAG_VERIFY(* this);
}

Transformation const & Object::GetModelViewTransformation() const
{
	return _model_view_transformation;
}

Transformation const & Object::GetShadowModelViewTransformation() const
{
	return _model_view_transformation;
}

bool gfx::operator < (Object const & lhs, Object const & rhs)
{
	if (lhs._layer == Layer::opaque)
	{
		if (rhs._layer == Layer::opaque)
		{
			ptrdiff_t mesh_index_diff = reinterpret_cast<char const *>(lhs._vbo_resource) - reinterpret_cast<char const *>(rhs._vbo_resource);
			if (mesh_index_diff != 0)
			{
				return mesh_index_diff < 0;
			}
			
			if (lhs._program != rhs._program)
			{
				return lhs._program < rhs._program;
			}
			
			return lhs._render_depth > rhs._render_depth;
		}

		return true;
	}
	else
	{
		if (rhs._layer == Layer::opaque)
		{
			return false;
		}
		
		return lhs._render_depth < rhs._render_depth;
	}
}

Layer Object::GetLayer() const
{
	return _layer;
}

ProgramHandle Object::GetProgram() const
{
	return _program;
}

void Object::SetProgram(ProgramHandle program)
{
	_program = program;

	CRAG_VERIFY(* this);
}

VboResource const * Object::GetVboResource() const
{
	return _vbo_resource;
}

void Object::SetVboResource(VboResource const * vbo_resource)
{
	_vbo_resource = vbo_resource;
}

bool Object::CastsShadow() const
{
	return _casts_shadow;
}

bool Object::GetRenderRange(RenderRange &) const 
{ 
	return false;
}

Object::PreRenderResult Object::PreRender()
{
	return ok;
}

bool Object::GenerateShadowVolume(Light const &, ShadowVolume &) const
{
	ASSERT(false);
	return true;
}

void Object::Render(Engine const &) const
{
	ASSERT(false);
}

//
//  Scene.cpp
//  crag
//
//  Created by John on 12/4/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Scene.h"

#include "core/ConfigEntry.h"

#include "gfx/object/Object.h"


using namespace gfx;


// used in CalculateNodeScoreFunctor.cpp
CONFIG_DEFINE (camera_near, float, .25f);


namespace
{
	CONFIG_DEFINE_ANGLE (camera_fov, float, 55.f);
}


////////////////////////////////////////////////////////////////////////////////
//

Scene::Scene(Engine & engine)
: _time(-1)
, _root(smp::ObjectInit<Engine>(engine, Uid::Create()), Transformation::Matrix44::Identity())
{
	Frustum & frustum = pov.GetFrustum();

	frustum.fov = static_cast<double>(camera_fov);
	frustum.depth_range[0] = frustum.depth_range[1] = -1;
}

Scene::~Scene()
{
	RemoveChildren(_root);

	ASSERT(_objects.empty());
	ASSERT(_root.IsEmpty());
}

#if defined(VERIFY)
void Scene::Verify() const
{
	VerifyObject(_root);
	
	for (ObjectMap::const_iterator i = _objects.begin(), end = _objects.end(); i != end; ++ i)
	{
		Object const & object = * i->second;
		VerifyObjectRef(object);
		VerifyTrue(object.GetParent() != nullptr);
	}
}
#endif

void Scene::SetTime(core::Time t)
{
	_time = t;
}

core::Time Scene::GetTime() const
{
	return _time;
}

void Scene::AddObject(Object & object)
{
	// add object to map
	Uid uid = object.GetUid();
	ASSERT(uid);

	ASSERT(_objects.count(uid) == 0);	// object with matching id already lives in map
	ObjectMap::value_type addition(uid, & object);
	_objects.insert(addition);
	ASSERT(_objects.count(uid) == 1);	// insertion failed somehow
	
	// If object is a LeafNode,
	LeafNode * leaf_node = object.CastLeafNodePtr();
	if (leaf_node != nullptr)
	{
		// add it to the render list.
		_render_list.push_back(* leaf_node);
	}
}

void Scene::RemoveObject(Uid uid)
{
	// Given the UID, get the object.
	Object * object;
	{
		ObjectMap::iterator i = _objects.find(uid);
		if (i == _objects.end())
		{
			// Probably, the object was already removed. 
			// (Children and sometimes parents are removed automatically.)
			// Possibly, it was simply never added with AddObject.
			ASSERT(false);
			return;
		}
		object = i->second;
		
		// And remove from the map.
		_objects.erase(i);
		ASSERT(_objects.count(uid) == 0);	// erasure failed somehow
	}
	
	switch (object->GetNodeType())
	{
		case Object::branch:
		{
			// If it's a branch,
			BranchNode & branch_node = object->CastBranchNodeRef();
			
			// then for all the children,
			Object::List::iterator end = branch_node.End();
			while (true)
			{
				Object::List::iterator last = end;
				-- last;
				if (last == end)
				{
					break;
				}
				
				// remove them first.
				Object & child_object = static_cast<Object &>(* last);
				Uid child_uid = child_object.GetUid();
				RemoveObject(child_uid);
			}
			
			ASSERT(branch_node.IsEmpty());
			break;
		}
			
		case Object::leaf:
		{
			LeafNode & leaf_node = object->CastLeafNodeRef();
			_render_list.remove(leaf_node);
			break;
		}
	}
	
	// and delete. (Object removes itself from parent list here.)
	delete object;
}

void Scene::SortRenderList()
{
	typedef LeafNode::RenderList List;
	List unsorted;
	
	std::swap(unsorted, _render_list);
	
	while (! unsorted.empty())
	{
		LeafNode & node = unsorted.front();
		unsorted.pop_front();
		
		List::iterator i = _render_list.begin();
		for (List::iterator end = _render_list.end(); i != end && node < * i; ++ i)
		{
		}
		
		_render_list.insert(i, node);
	}
}

Object * Scene::GetObject(Uid object_uid)
{
	ObjectMap::iterator i = _objects.find(object_uid);
	if (i == _objects.end())
	{
		return nullptr;
	}
	
	return i->second;
}

Object const * Scene::GetObject(Uid object_uid) const
{
	ObjectMap::const_iterator i = _objects.find(object_uid);
	if (i == _objects.end())
	{
		return nullptr;
	}
	
	return i->second;
}

BranchNode & Scene::GetRoot()
{
	return _root;
}

BranchNode const & Scene::GetRoot() const
{
	return _root;
}

LeafNode::RenderList & Scene::GetRenderList()
{
	return _render_list;
}

LeafNode::RenderList const & Scene::GetRenderList() const
{
	return _render_list;
}

Light::List & Scene::GetLightList()
{
	return _light_list;
}

void Scene::SetResolution(geom::Vector2i const & r)
{
	pov.GetFrustum().resolution = r;
}

void Scene::SetCameraTransformation(sim::Transformation const & transformation)
{
	pov.SetTransformation(transformation);
	_root.SetTransformation(transformation.GetInverse());
}

Pov & Scene::GetPov()
{
	return pov;
}

Pov const & Scene::GetPov() const
{
	return pov;
}

void Scene::RemoveChildren(BranchNode & parent)
{
	// then for all the children,
	Object::List::iterator end = parent.End();
	while (true)
	{
		Object::List::iterator last = end;
		-- last;
		if (last == end)
		{
			break;
		}
		
		// remove them first.
		Object & child_object = static_cast<Object &>(* last);
		Uid child_uid = child_object.GetUid();
		RemoveObject(child_uid);
	}
	
	ASSERT(parent.IsEmpty());
}

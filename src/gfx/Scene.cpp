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

#include "SphereMesh.h"
#include "Cuboid.h"

#include "core/ConfigEntry.h"

#include "gfx/object/Object.h"

#include <algorithm>


using namespace gfx;


// used in CalculateNodeScoreFunctor.cpp
CONFIG_DEFINE (camera_near, float, .25f);


namespace
{
	CONFIG_DEFINE_ANGLE (camera_fov, float, 55.f);
	CONFIG_DEFINE (camera_far, float, 10);
}


////////////////////////////////////////////////////////////////////////////////
//

Scene::Scene()
: _time(-1)
, _cuboid(* new Cuboid)
, _sphere(* new SphereMesh)
{
	Frustum & frustum = pov.GetFrustum();

	frustum.fov = static_cast<double>(camera_fov);
	frustum.depth_range[0] = static_cast<double>(camera_near);
	frustum.depth_range[1] = static_cast<double>(camera_far);
}

Scene::~Scene()
{
	RemoveChildren(_root);

	Assert(_objects.empty());
	Assert(_root.IsEmpty());
	
	Frustum const & frustum = pov.GetFrustum();
	
	camera_fov = static_cast<float>(frustum.fov);
	camera_near = static_cast<float>(frustum.depth_range[0]);
	camera_far = static_cast<float>(frustum.depth_range[1]);
	
	delete & _sphere;
	delete & _cuboid;
}

void Scene::SetTime(Time t)
{
	_time = t;
}

Time Scene::GetTime() const
{
	return _time;
}

void Scene::AddObject(Object & object, Uid parent_uid)
{
	// determine the parent from the parent_uid
	BranchNode * parent;
	if (parent_uid == Uid::null)
	{
		parent = & _root;
	}
	else
	{
		ObjectMap::iterator i = _objects.find(parent_uid);
		if (i == _objects.end())
		{
			Assert(false);
			return;
		}
		
		parent = static_cast<BranchNode *>(i->second);
	}
	
	// add object to its parent's list
	VerifyObject(* parent);
	parent->AddChild(object);
	
	// add object to map
	Uid uid = object.GetUid();
	Assert(_objects.count(uid) == 0);	// object with matching id already lives in map
	ObjectMap::value_type addition(uid, & object);
	_objects.insert(addition);
	Assert(_objects.count(uid) == 1);	// insertion failed somehow
	
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
			Assert(false);
			return;
		}
		object = i->second;
		
		// And remove from the map.
		_objects.erase(i);
		Assert(_objects.count(uid) == 0);	// erasure failed somehow
	}
	
	switch (object->GetNodeType())
	{
		case Object::branch:
		{
			// If it's a branch,
			BranchNode & branch_node = object->CastBranchNodeRef();
			
			// then for all the children,
			ObjectBase::ChildList::iterator end = branch_node.End();
			while (true)
			{
				ObjectBase::ChildList::iterator last = end;
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
			
			Assert(branch_node.IsEmpty());
			break;
		}
			
		case Object::leaf:
		{
			LeafNode & leaf_node = object->CastLeafNodeRef();
			_render_list.remove(leaf_node);
			break;
		}
	}
	
	// Finally, deinitialize ...
	object->Deinit();
	
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

ObjectMap & Scene::GetObjectMap()
{
	return _objects;
}

ObjectMap const & Scene::GetObjectMap() const
{
	return _objects;
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

void Scene::SetResolution(Vector2i const & r)
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

gfx::SphereMesh const & Scene::GetSphere() const
{
	return _sphere;
}

Cuboid const & Scene::GetCuboid() const
{
	return _cuboid;
}

void Scene::RemoveChildren(BranchNode & parent)
{
	// then for all the children,
	ObjectBase::ChildList::iterator end = parent.End();
	while (true)
	{
		ObjectBase::ChildList::iterator last = end;
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
	
	Assert(parent.IsEmpty());
}

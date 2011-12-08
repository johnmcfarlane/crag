/*
 *  Scene.cpp
 *  Crag
 *
 *  Created by John on 12/4/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Scene.h"

#include "Sphere.h"
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
: _cuboid(* new Cuboid)
, _sphere(* new Sphere)
{
	Frustum & frustum = pov.GetFrustum();

	frustum.fov = static_cast<double>(camera_fov);
	frustum.depth_range[1] = static_cast<double>(camera_near);
	frustum.depth_range[0] = static_cast<double>(camera_far);
}

Scene::~Scene()
{
	Assert(_objects.empty());
	Assert(_root.IsEmpty());
	
	Frustum const & frustum = pov.GetFrustum();
	
	camera_fov = static_cast<float>(frustum.fov);
	camera_near = static_cast<float>(frustum.depth_range[1]);
	camera_far = static_cast<float>(frustum.depth_range[0]);
	
	delete & _sphere;
	delete & _cuboid;
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
}

void Scene::RemoveObject(Uid uid)
{
	// Given the UID, get the object.
	Object * object;
	{
		ObjectMap::iterator i = _objects.find(uid);
		if (i == _objects.end())
		{
			Assert(false);
			return;
		}
		object = i->second;

		// And remove from the map.
		_objects.erase(i);
		Assert(_objects.count(uid) == 0);	// erasure failed
	}
	
	// If it's a branch,
	BranchNode * branch_node = object->CastBranchNodePtr();
	if (branch_node != nullptr)
	{
		// then for all the children,
		Object::ChildList::iterator end = branch_node->End();
		while (true)
		{
			Object::ChildList::iterator last = end;
			-- last;
			if (last == end)
			{
				break;
			}
			
			// remove them first.
			Object & child_object = * last;
			Uid child_uid = child_object.GetUid();
			RemoveObject(child_uid);
		}
		
		Assert(branch_node->IsEmpty());
	}
	
	// Finally, deinitialize ...
	object->Deinit();

	// and delete. (Object removes itself from parent list here.)
	delete object;
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

gfx::Sphere const & Scene::GetSphere() const
{
	return _sphere;
}

Cuboid const & Scene::GetCuboid() const
{
	return _cuboid;
}

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
	frustum.near_z = static_cast<double>(camera_near);
	frustum.far_z = static_cast<double>(camera_far);
}

Scene::~Scene()
{
	Assert(_objects.empty());
	Assert(_root.IsEmpty());
	
	Frustum const & frustum = pov.GetFrustum();
	
	camera_fov = static_cast<float>(frustum.fov);
	camera_near = static_cast<float>(frustum.near_z);
	camera_far = static_cast<float>(frustum.far_z);
	
	delete & _sphere;
	delete & _cuboid;
}

void Scene::AddObject(Uid uid, Object & object, Uid parent_uid)
{
	// object with matching id already lives in map
	Assert(_objects.count(uid) == 0);
	
	// add to tree
	if (parent_uid == Uid())
	{
		_root.AddChild(object);
	}
	else
	{
		ObjectMap::iterator i = _objects.find(parent_uid);
		if (i == _objects.end())
		{
			// Parent already removed? If so, ok.
			Assert(false);
			return;
		}
		
		BranchNode & parent = static_cast<BranchNode &>(* i->second);
		VerifyObject(parent);
	}

	// add to map
	ObjectMap::value_type addition(uid, & object);
	_objects.insert(addition);
}

void Scene::RemoveObject(Uid uid)
{
	ObjectMap::iterator i = _objects.find(uid);
	if (i == _objects.end())
	{
		Assert(false);
		return;
	}
	
	Object * object = i->second;

	_objects.erase(uid);

	object->Deinit();
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

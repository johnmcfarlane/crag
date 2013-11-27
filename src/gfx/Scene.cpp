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
CONFIG_DEFINE_ANGLE (camera_fov, float, 55.f);

////////////////////////////////////////////////////////////////////////////////
// Scene member functions

Scene::Scene(Engine & engine)
: _time(-1)
, _root(ipc::ObjectInit<Engine>(engine, Uid::Create()), Transformation::Matrix44::Identity())
{
	auto frustum = pov.GetFrustum();

	frustum.fov = camera_fov;
	frustum.depth_range[0] = frustum.depth_range[1] = -1;
	
	pov.SetFrustum(frustum);
}

Scene::~Scene()
{
	ASSERT(_root.IsEmpty());
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Scene, self)
	CRAG_VERIFY(self._root);
CRAG_VERIFY_INVARIANTS_DEFINE_END

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
	CRAG_VERIFY(object);

	// If object is a LeafNode,
	LeafNode * leaf_node = object.CastLeafNodePtr();
	if (leaf_node != nullptr)
	{
		// add it to the render list.
		_render_list.push_back(* leaf_node);
	}

	AdoptChild(object, _root);
}

void Scene::RemoveObject(Object & object)
{
	LeafNode * leaf_node = object.CastLeafNodePtr();
	if (leaf_node == nullptr)
	{
		return;
	}

	_render_list.remove(* leaf_node);
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

Object & Scene::GetRoot()
{
	return _root;
}

Object const & Scene::GetRoot() const
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
	auto frustum = pov.GetFrustum();
	frustum.resolution = r;
	pov.SetFrustum(frustum);
}

void Scene::SetCameraTransformation(Transformation const & transformation)
{
	pov.SetTransformation(transformation);
	_root.SetLocalTransformation(Inverse(transformation.GetMatrix()));
}

Pov & Scene::GetPov()
{
	return pov;
}

Pov const & Scene::GetPov() const
{
	return pov;
}

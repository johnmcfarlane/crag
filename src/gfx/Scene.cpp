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
#include "ShadowVolume.h"

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
	ASSERT(_root.GetChildren().empty());
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Scene, self)
	CRAG_VERIFY(self._root);
	auto num_shadow_casters = std::count_if(std::begin(self._render_list), std::end(self._render_list), [] (LeafNode const & object)
	{
		return object.CastsShadow();
	});
	auto num_shadow_lights = std::count_if(std::begin(self._light_list), std::end(self._light_list), [] (Light const & light)
	{
		return light.MakesShadow();
	});
	CRAG_VERIFY_OP(self._shadows.size(), <=, unsigned(num_shadow_casters * num_shadow_lights));
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
	if (leaf_node == nullptr)
	{
		return;
	}
	
	// add it to the render list.
	_render_list.push_back(* leaf_node);

	AdoptChild(object, _root);
	
	if (leaf_node->CastsShadow())
	{
		std::for_each(std::begin(_light_list), std::end(_light_list), [&] (Light & light)
		{
			auto key = std::make_pair(leaf_node, & light);
			ASSERT(_shadows.find(key) == std::end(_shadows));
			
			if (light.MakesShadow())
			{
				_shadows.insert(std::make_pair(key, ShadowVolume()));
			}
		});
	}

	CRAG_VERIFY(* this);
}

void Scene::RemoveObject(Object & object)
{
	LeafNode * leaf_node = object.CastLeafNodePtr();
	if (leaf_node == nullptr)
	{
		return;
	}

	// remove from list of things to be drawn
	_render_list.remove(* leaf_node);
	
	// remove from list of things that cast a shadow
	if (leaf_node->CastsShadow())
	{
		std::for_each(std::begin(_light_list), std::end(_light_list), [&] (Light & light)
		{
			auto key = std::make_pair(leaf_node, & light);
			if (light.MakesShadow())
			{
				ASSERT(_shadows.find(key) != std::end(_shadows));
			
				_shadows.erase(key);
			}
		});
	}
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
		
		auto insertion_point = std::lower_bound(std::begin(_render_list), std::end(_render_list), node);
		_render_list.insert(insertion_point, node);
	}
}

void Scene::AddLight(Light & light)
{
	CRAG_VERIFY(* this);
	
	ASSERT(! _light_list.contains(light));
	_light_list.push_back(light);
	
	if (light.MakesShadow())
	{
		for (auto & object : _render_list)
		{
			if (! object.CastsShadow())
			{
				continue;
			}
			
			auto key = std::make_pair(& object, & light);
			ASSERT(_shadows.find(key) == std::end(_shadows));
			_shadows.insert(std::make_pair(key, ShadowVolume()));
		}
	}

	CRAG_VERIFY(light);
	CRAG_VERIFY(* this);
}

void Scene::RemoveLight(Light & light)
{
	CRAG_VERIFY(light);

	ASSERT(_light_list.contains(light));
	_light_list.remove(light);

	if (light.MakesShadow())
	{
		for (auto & object : _render_list)
		{
			if (! object.CastsShadow())
			{
				continue;
			}
		
			// find the shadow volume resource matching this object/light pair
			auto key = std::make_pair(& object, & light);
			ASSERT(_shadows.find(key) != std::end(_shadows));
		
			_shadows.erase(key);
		}
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

Light::List const & Scene::GetLightList() const
{
	return _light_list;
}

ShadowMap & Scene::GetShadows()
{
	return _shadows;
}

ShadowMap const & Scene::GetShadows() const
{
	return _shadows;
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

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

////////////////////////////////////////////////////////////////////////////////
// Scene member functions

Scene::Scene(Engine & engine)
: _time(-1)
, _root(engine, Transformation::Matrix44::Identity(), Layer::background)
{
}

Scene::~Scene()
{
	ASSERT(_root.GetChildren().empty());
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Scene, self)
	CRAG_VERIFY(self._root);
	for (auto object : self._render_list)
	{
		CRAG_VERIFY_REF(* object);
	}
	auto num_shadow_casters = std::count_if(std::begin(self._render_list), std::end(self._render_list), [] (Object const * object)
	{
		return object->CastsShadow();
	});
	auto num_shadow_lights = std::count_if(std::begin(self._light_list), std::end(self._light_list), [] (Light const & light)
	{
		return light.GetAttributes().makes_shadow;
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
	ASSERT (& object != & _root);
	
	// add it to the render list.
	_render_list.push_back(& object);

	AdoptChild(object, _root);
	
	if (object.CastsShadow())
	{
		std::for_each(std::begin(_light_list), std::end(_light_list), [&] (Light & light)
		{
			if (light.GetException() == & object)
			{
				return;
			}
			
			auto key = std::make_pair(& object, & light);
			ASSERT(_shadows.find(key) == std::end(_shadows));
			
			if (light.GetAttributes().makes_shadow)
			{
				_shadows.insert(std::make_pair(key, ShadowVolumeResource::Create<ShadowVolume>([] () {
					return ShadowVolume();
				})));
			}
		});
	}

	CRAG_VERIFY(* this);
}

void Scene::RemoveObject(Object & object)
{
	ASSERT (& object != & _root);

	// remove from list of things to be drawn; if slow, consider bsearch exploiting sortedness
	auto found = std::find(std::begin(_render_list), std::end(_render_list), & object);
	_render_list.erase(found);
	
	// remove from list of things that cast a shadow
	if (object.CastsShadow())
	{
		std::for_each(std::begin(_light_list), std::end(_light_list), [&] (Light & light)
		{
			auto key = std::make_pair(& object, & light);
			if (light.GetAttributes().makes_shadow && light.GetException() != & object)
			{
				ASSERT(_shadows.find(key) != std::end(_shadows));
			
				_shadows.erase(key);
			}
			else
			{
				ASSERT(_shadows.find(key) == std::end(_shadows));
			}
		});
	}
}

void Scene::SortRenderList()
{
	std::sort(std::begin(_render_list), std::end(_render_list));
}

void Scene::AddLight(Light & light)
{
	CRAG_VERIFY(* this);
	
	ASSERT(! _light_list.contains(light));
	_light_list.push_back(light);
	
	if (light.GetAttributes().makes_shadow)
	{
		for (auto object : _render_list)
		{
			if (! object->CastsShadow())
			{
				continue;
			}
			
			if (light.GetException() == object)
			{
				return;
			}
			
			auto key = std::make_pair(object, & light);
			ASSERT(_shadows.find(key) == std::end(_shadows));
			_shadows.insert(std::make_pair(key, ShadowVolumeResource::Create<ShadowVolume>([] () {
				return ShadowVolume();
			})));
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

	if (light.GetAttributes().makes_shadow)
	{
		for (auto object : _render_list)
		{
			if (! object->CastsShadow() || light.GetException() == object)
			{
				continue;
			}
		
			// find the shadow volume resource matching this object/light pair
			auto key = std::make_pair(object, & light);
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

Scene::RenderList & Scene::GetRenderList()
{
	return _render_list;
}

Scene::RenderList const & Scene::GetRenderList() const
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

void Scene::SetPov(Pov const & pov)
{
	_pov = pov;
	_root.SetLocalTransformation(Inverse(_pov.GetTransformation().GetMatrix()));
}

Pov & Scene::GetPov()
{
	return _pov;
}

Pov const & Scene::GetPov() const
{
	return _pov;
}

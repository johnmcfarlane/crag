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

#include "core/ConfigEntry.h"

#include "gfx/Object.h"

#include <algorithm>


CONFIG_DEFINE_ANGLE (camera_fov, float, 55.f);
CONFIG_DEFINE (camera_near, float, .25f);
CONFIG_DEFINE (camera_far, float, 10);


gfx::Scene::Scene()
: _sphere(* new Sphere)
{
	pov.frustum.fov = static_cast<double>(camera_fov);
	pov.frustum.near_z = static_cast<double>(camera_near);
	pov.frustum.far_z = static_cast<double>(camera_far);
}

gfx::Scene::~Scene()
{
	for (ObjectSet * it = _objects; it != _objects + Layer::num; ++ it)
	{
		Assert(it->empty());
	}
	
	camera_fov = static_cast<float>(pov.frustum.fov);
	camera_near = static_cast<float>(pov.frustum.near_z);
	camera_far = static_cast<float>(pov.frustum.far_z);
	
	delete & _sphere;
}

void gfx::Scene::AddObject(Object const & object)
{
	for (Layer::type layer = Layer::begin; layer < Layer::end; ++ layer)
	{
		if (object.IsInLayer(layer))
		{
			ObjectSet & objects = _objects[layer];
			
			Assert(objects.count(& object) == 0);
			objects.insert(& object);
		}
	}
}

void gfx::Scene::RemoveObject(Object const & object)
{
	for (Layer::type layer = Layer::begin; layer < Layer::end; ++ layer)
	{
		if (object.IsInLayer(layer))
		{
			ObjectSet & objects = _objects[layer];
			
			Assert(objects.count(& object) == 1);
			objects.erase(& object);
		}
	}
}

gfx::ObjectSet const & gfx::Scene::GetObjects(Layer::type layer) const
{
	return _objects[layer];
}

void gfx::Scene::SetResolution(Vector2i const & r)
{
	pov.frustum.resolution = r;
}

void gfx::Scene::SetCamera(sim::Vector3 const & pos, sim::Matrix4 const & rot)
{
	pov.pos = pos;
	pov.rot = rot;
}

gfx::Pov & gfx::Scene::GetPov()
{
	return pov;
}

gfx::Pov const & gfx::Scene::GetPov() const
{
	return pov;
}

// Given a camera position/direction, conservatively estimates 
// the minimum and maximum distances at which rendering occurs.
// TODO: Long-term, this function needs to be replaced with 
// something that gives and near and far plane value instead. 
void gfx::Scene::GetRenderRange(sim::Ray3 const & camera_ray, double & range_min, double & range_max, bool wireframe) const
{
	for (ObjectSet const * vector_iterator = _objects; vector_iterator != _objects + Layer::num; ++ vector_iterator) 
	{
		ObjectSet objects = * vector_iterator;
		for (ObjectSet::const_iterator object_iterator = objects.begin(); object_iterator != objects.end(); ++ object_iterator) 
		{
			Object const & object = * * object_iterator;
			double object_range[2];
			if (object.GetRenderRange(camera_ray, object_range, wireframe))
			{
				if (object_range[0] < range_min)
				{
					range_min = object_range[0];
				}
				
				if (object_range[1] > range_max)
				{
					range_max = object_range[1];
				}
			}
		}
	}
}

gfx::Sphere const & gfx::Scene::GetSphere() const
{
	return _sphere;
}

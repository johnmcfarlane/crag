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

#include "sim/Entity.h"

#include "core/ConfigEntry.h"

#include <algorithm>


CONFIG_DEFINE_ANGLE (camera_fov, float, 55.f);
CONFIG_DEFINE (camera_near, float, .25f);
CONFIG_DEFINE (camera_far, float, 10);


gfx::Scene::Scene()
: skybox(nullptr)
, _sphere(* new Sphere)
{
	pov.frustum.fov = static_cast<double>(camera_fov);
	pov.frustum.near_z = static_cast<double>(camera_near);
	pov.frustum.far_z = static_cast<double>(camera_far);
}

gfx::Scene::~Scene()
{
	Assert(entities.empty());
	
	camera_fov = static_cast<float>(pov.frustum.fov);
	camera_near = static_cast<float>(pov.frustum.near_z);
	camera_far = static_cast<float>(pov.frustum.far_z);

	delete & _sphere;
}

void gfx::Scene::AddLight(Light const & light)
{
	Assert(std::find(lights.begin(), lights.end(), & light) == lights.end());
	lights.push_back(& light);
}

void gfx::Scene::AddEntity(sim::Entity const & entity)
{
	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
	entities.push_back(& entity);
}

void gfx::Scene::RemoveEntity(sim::Entity const & entity)
{
	EntityVector::iterator i = std::find(entities.begin(), entities.end(), & entity);
	Assert(i != entities.end());
	entities.erase(i);

	Assert(std::find(entities.begin(), entities.end(), & entity) == entities.end());
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
	for (EntityVector::const_iterator it = entities.begin(); it != entities.end(); ++ it) 
	{
		sim::Entity const & e = * * it;
		double entity_range[2];
		if (e.GetRenderRange(camera_ray, entity_range, wireframe))
		{
			if (entity_range[0] < range_min)
			{
				range_min = entity_range[0];
			}
			
			if (entity_range[1] > range_max)
			{
				range_max = entity_range[1];
			}
		}
	}
}

gfx::Sphere const & gfx::Scene::GetSphere() const
{
	return _sphere;
}

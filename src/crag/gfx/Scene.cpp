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
#include "core/ConfigEntry.h"

#include <algorithm>


CONFIG_DEFINE_ANGLE (camera_fov, float, 55.f);
CONFIG_DEFINE (camera_near, float, .05f);
CONFIG_DEFINE (camera_far, float, 1000000);


gfx::Scene::Scene()
: skybox(nullptr)
{
	pov.frustum.fov = static_cast<double>(camera_fov);
	pov.frustum.near_z = static_cast<double>(camera_near);
	pov.frustum.far_z = static_cast<double>(camera_far);
}

gfx::Scene::~Scene()
{
	camera_fov = static_cast<float>(pov.frustum.fov);
	camera_near = static_cast<float>(pov.frustum.near_z);
	camera_far = static_cast<float>(pov.frustum.far_z);
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


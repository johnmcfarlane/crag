//
//  gfx/object/Puff.cpp
//  crag
//
//  Created by John McFarlane on 2011-12-10.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Puff.h"

#include "gfx/Program.h"
#include "gfx/Engine.h"
#include "gfx/Scene.h"
#include "gfx/Quad.h"

#include "geom/origin.h"
#include "geom/Sphere.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"
#include "core/ResourceManager.h"
#include "core/Statistics.h"


using namespace gfx;


namespace
{
	CONFIG_DEFINE(puff_displacement, float, .75);	// m/s
	CONFIG_DEFINE(puff_radius_growth_rate, float, 2.5f);	// m/s
	CONFIG_DEFINE(puff_radius_coefficient, float, 3.5);
	CONFIG_DEFINE(puff_min_alpha, float, .01f);
	
	STAT(num_puffs, int, .1f);
}

DEFINE_POOL_ALLOCATOR(Puff, 100);

Puff::Puff(Init const & init, Transformation const & local_transformation, Scalar spawn_volume)
: Object(init, local_transformation, Layer::transparent)
, _spawn_volume(spawn_volume)
, _radius(0)
, _color(0.75, 0.75, 0.75, 1)
{
	STAT_INC(num_puffs, 1);

	auto const & resource_manager = crag::core::ResourceManager::Get();
	
	auto const & program = * resource_manager.GetHandle<DiskProgram>("DiskProgram");
	SetProgram(& program);
	
	auto const & disk_quad = * resource_manager.GetHandle<Quad>("QuadVbo");
	SetVboResource(& disk_quad);
	
	Scene const & scene = init.engine.GetScene();
	core::Time time = scene.GetTime();
	_spawn_time = time - (Random::sequence.GetUnit<double>() / 60.);
}

Puff::~Puff()
{
	STAT_INC(num_puffs, -1);
}

void Puff::UpdateModelViewTransformation(Transformation const & model_view)
{
	core::Time time = GetEngine().GetScene().GetTime();
	core::Time age = CalculateAge(time);
	
	Transformation scale = model_view * Transformation(Vector3(geom::rel::Scalar(age * puff_displacement), 0., 0.), Matrix33::Identity(), Scalar(_radius));
	
	Quad const & disk_quad = static_cast<Quad const &>(* GetVboResource());
	SetModelViewTransformation(disk_quad.CalculateModelViewTransformation(scale, _radius));
}

Object::PreRenderResult Puff::PreRender()
{
	core::Time time = GetEngine().GetScene().GetTime();
	core::Time age = CalculateAge(time);
	
	_radius = static_cast<float>(geom::Sphere<Scalar, 3>::Properties::RadiusFromVolume(_spawn_volume));
	_radius += puff_radius_growth_rate * static_cast<float>(age);
	
	_color.a = std::min(1.f / Squared(_radius * puff_radius_coefficient), 1.f);
	
	if (_color.a < puff_min_alpha)
	{
		return remove;
	}

	return ok;
}

void Puff::Render(Engine const &) const
{
	// Pass rendering details to the shader program.
	DiskProgram const & disk_program = static_cast<DiskProgram const &>(* GetProgram());
	Transformation const & model_view = GetModelViewTransformation();
	disk_program.SetUniforms(model_view, _radius, _color);
	
	// Draw the ball.
	Quad const & disk_quad = static_cast<Quad const &>(* GetVboResource());
	disk_quad.Draw();
}

core::Time Puff::CalculateAge(core::Time time) const
{
	core::Time age = time - _spawn_time;
	return age;
}

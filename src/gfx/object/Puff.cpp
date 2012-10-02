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
#include "gfx/ResourceManager.h"
#include "gfx/Scene.h"
#include "gfx/Quad.h"

#include "geom/Sphere.h"

#include "core/ConfigEntry.h"
#include "core/Random.h"
#include "core/Statistics.h"


using namespace gfx;


namespace
{
	CONFIG_DEFINE(puff_displacement, double, .75);	// m/s
	CONFIG_DEFINE(puff_radius_growth_rate, float, 2.5f);	// m/s
	CONFIG_DEFINE(puff_radius_coefficient, float, 3.5);
	CONFIG_DEFINE(puff_min_alpha, float, .01f);
	
	STAT(num_puffs, int, .1f);
}


Puff::Puff(LeafNode::Init const & init, Scalar spawn_volume)
: LeafNode(init, Layer::foreground)
, _spawn_volume(spawn_volume)
, _radius(0)
, _color(0.75, 0.75, 0.75, 1)
{
	SetIsOpaque(false);
	STAT_INC(num_puffs, 1);

	ResourceManager & resource_manager = init.engine.GetResourceManager();
	
	Program const * poly_program = resource_manager.GetProgram(ProgramIndex::disk);
	SetProgram(poly_program);
	
	MeshResource const & disk_quad = resource_manager.GetDiskQuad();
	SetMeshResource(& disk_quad);
	
	Scene const & scene = init.engine.GetScene();
	core::Time time = scene.GetTime();
	_spawn_time = time - (Random::sequence.GetUnit<double>() / 60.);
}

Puff::~Puff()
{
	STAT_INC(num_puffs, -1);
}

gfx::Transformation const & Puff::Transform(gfx::Engine & renderer, gfx::Transformation const & model_view, gfx::Transformation & scratch) const
{
	core::Time time = renderer.GetScene().GetTime();
	core::Time age = CalculateAge(time);
	
	gfx::Transformation scale = model_view * gfx::Transformation(Vector3(age * puff_displacement, 0., 0.), Matrix33::Identity(), _radius);
	
	Quad const & disk_quad = static_cast<Quad const &>(* GetMeshResource());
	return disk_quad.Transform(scale, scratch);
}

LeafNode::PreRenderResult Puff::PreRender()
{
	core::Time time = GetEngine().GetScene().GetTime();
	core::Time age = CalculateAge(time);
	
	_radius = static_cast<float>(geom::Sphere<Scalar, 3>::Properties::RadiusFromVolume(_spawn_volume));
	_radius += puff_radius_growth_rate * static_cast<float>(age);
	
	_color.a = std::min(1.f / Square(_radius * puff_radius_coefficient), 1.f);
	
	if (_color.a < puff_min_alpha)
	{
		return remove;
	}

	return ok;
}

void Puff::Render(gfx::Engine const & renderer) const
{
	DiskProgram const & disk_program = static_cast<DiskProgram const &>(ref(GetProgram()));
	Transformation const & model_view = GetModelViewTransformation();
	Vector3 translation = model_view.GetTranslation();
	Color4f lighting = renderer.CalculateLighting(translation);
	disk_program.SetUniforms(model_view, _color * lighting);
	
	Quad const & disk_quad = static_cast<Quad const &>(ref(GetMeshResource()));
	disk_quad.Draw();
}

core::Time Puff::CalculateAge(core::Time time) const
{
	core::Time age = time - _spawn_time;
	return age;
}

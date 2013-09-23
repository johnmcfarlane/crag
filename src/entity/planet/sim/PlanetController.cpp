//
//  PlanetController.cpp
//  crag
//
//  Created by john on 4/14/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "PlanetController.h"

#include "MoonShader.h"
#include "PlanetShader.h"

#include "entity/planet/gfx/Planet.h"
#include "entity/planet/physics/PlanetBody.h"

#include "sim/Entity.h"
#include "sim/Engine.h"

#include "form/Formation.h"

#include "gfx/Engine.h"

#include "core/Random.h"
#include "core/Roster.h"

using namespace sim;

//////////////////////////////////////////////////////////////////////
// sim::PlanetController member definitions

DEFINE_POOL_ALLOCATOR(PlanetController, 3);

PlanetController::PlanetController(Entity & entity, Sphere3 const & sphere, int random_seed, int num_craters)
: Controller(entity)
, _formation(nullptr)
, _radius_mean(sphere.radius)
{
	ASSERT(sphere.radius > 0);

	Random random(random_seed);
	
	// factory
	form::Shader * shader;
	if (num_craters > 0)
	{
		int random_seed_shader = random.GetInt();
		shader = new MoonShader(random_seed_shader, num_craters, _radius_mean);
	}
	else 
	{
		shader = new PlanetShader();
	}
	
	// formation
	auto & engine = entity.GetEngine();
	int random_seed_formation = random.GetInt();
	geom::abs::Sphere3 formation_sphere = geom::Cast<geom::abs::Scalar>(sphere);
	_formation = new form::Formation(random_seed_formation, * shader, formation_sphere);
	engine.AddFormation(* _formation);

	// roster
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.AddOrdering(& PlanetController::Tick, & Entity::Tick);
	roster.AddCommand(* this, & PlanetController::Tick);
}

PlanetController::~PlanetController()
{
	auto& engine = GetEntity().GetEngine();

	// roster
	auto & roster = engine.GetTickRoster();
	roster.RemoveCommand(* this, & PlanetController::Tick);

	// unregister with formation manager
	engine.RemoveFormation(* _formation);
	_formation = nullptr;
}

form::Formation const & PlanetController::GetFormation() const
{
	return ref(_formation);
}

void PlanetController::Tick()
{
	auto radius_range = geom::Cast<gfx::Scalar>(_formation->GetRadiusRange());

	auto & entity = GetEntity();
	auto & body = ref(entity.GetBody());
	auto & planetary_body = static_cast<physics::PlanetBody &>(body);
	planetary_body.SetRadius(physics::Scalar(radius_range[1]));

	// update planet params
	auto model_handle = entity.GetModel();
	model_handle.Call([radius_range] (gfx::Object & object) {
		auto & planet = static_cast<gfx::Planet &>(object);
		planet.SetRadiusMinMax(radius_range[0], radius_range[1]);
	});
}

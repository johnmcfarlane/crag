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

#include "entity/gfx/Planet.h"
#include "entity/physics/PlanetBody.h"

#include "sim/Entity.h"
#include "sim/Engine.h"
#include "sim/Model.h"

#include "form/Engine.h"
#include "form/Object.h"

#include "gfx/Engine.h"

#include "core/RosterObjectDefine.h"

using namespace sim;

//////////////////////////////////////////////////////////////////////
// sim::PlanetController member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	PlanetController,
	10,
	Pool::Call<& PlanetController::Tick>(Engine::GetTickRoster()))

PlanetController::PlanetController(Entity & entity, Sphere3 const & sphere, int random_seed, int num_craters)
: Controller(entity)
, _radius_mean(sphere.radius)
{
	ASSERT(sphere.radius > 0);

	Random random(random_seed);
	
	// factory
	form::Formation::ShaderPtr shader;
	if (num_craters > 0)
	{
		int random_seed_shader = random.GetInt();
		shader = std::make_shared<MoonShader>(random_seed_shader, num_craters, _radius_mean);
	}
	else 
	{
		shader = std::make_shared<PlanetShader>();
	}
	
	// formation
	auto & engine = entity.GetEngine();
	int random_seed_formation = random.GetInt();
	geom::abs::Sphere3 formation_sphere = geom::Cast<geom::abs::Scalar>(sphere);
	
	_formation = FormationPtr(new form::Formation(random_seed_formation, shader, formation_sphere));
	_handle.CreateObject(* _formation);
	engine.AddFormation(* _formation);
}

PlanetController::~PlanetController()
{
	auto& engine = GetEntity().GetEngine();

	// remove physics
	engine.RemoveFormation(* _formation);

	// unregister with formation manager
	_handle.Release();
}


form::Formation const & PlanetController::GetFormation() const
{
	return * _formation;
}

void PlanetController::Tick()
{
	auto max_radius = _formation->GetMaxRadius();
	auto & entity = GetEntity();

#if defined(CRAG_SIM_FORMATION_PHYSICS)
	auto const & location = entity.GetLocation();
	auto & body = core::StaticCast<physics::Body const>(* location);
	auto & planetary_body = static_cast<physics::PlanetBody const &>(body);
	planetary_body.SetRadius(physics::Scalar(max_radius));
#endif

	// update planet params
	auto model_handle = entity.GetModel()->GetHandle();
	model_handle.Call([max_radius] (gfx::Object & object) {
		auto & planet = core::StaticCast<gfx::Planet>(object);
		planet.SetMaxRadius(static_cast<Scalar>(max_radius));
	});
}

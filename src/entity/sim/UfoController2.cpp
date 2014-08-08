//
//  entity/sim/UfoController2.cpp
//  crag
//
//  Created by John McFarlane on 2014-07-23.
//	Copyright 2014 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "UfoController2.h"
#include "RoverThruster.h"

#include "sim/Entity.h"
#include "sim/Engine.h"
#include "sim/gravity.h"

#include "physics/Body.h"

#include "gfx/axes.h"
#include "gfx/Debug.h"

#include "geom/Intersection.h"

#include "core/ConfigEntry.h"
#include "core/Roster.h"

using namespace sim;

CONFIG_DECLARE(saucer_ball_radius, physics::Scalar);

namespace
{
	CONFIG_DEFINE(ufo_controller2_sensitivity, Scalar, 5000.f);
	CONFIG_DEFINE(ufo_controller2_vanish_distance, Scalar, 1000000.f);
	CONFIG_DEFINE(ufo_controller2_drag_roof_height, Scalar, 7.5f);
	CONFIG_DEFINE(ufo_controller2_max_direction, Scalar, 7.5f);
	
	Ray3 GetPointerRay(gfx::Pov const & pov, geom::Vector2i const & pixel_position)
	{
		auto const & eye_transformation = pov.GetTransformation();
		auto const & eye_translation = eye_transformation.GetTranslation();
		auto const & screen_position = pov.PixelToWorld(geom::Cast<Scalar>(pixel_position));
		return Ray3(eye_translation, screen_position - eye_translation);
	}
	
	Vector3 GetWorldPosition(Ray3 const & pointer_ray, Plane3 const & horizontal_plane)
	{
		auto const & intersection = geom::GetIntersection(horizontal_plane, pointer_ray);
		if (intersection < 0)
		{
			return geom::Project(pointer_ray, ufo_controller2_vanish_distance);
		}
		
		return geom::Project(pointer_ray, intersection);
	}

	Vector3 GetTargetPosition(
		gfx::Pov const & pov, 
		geom::Vector2i const & pixel_position, 
		Plane3 const & horizontal_plane)
	{
		// pointer ray
		auto const & pointer_ray = GetPointerRay(pov, pixel_position);
	
		return GetWorldPosition(pointer_ray, horizontal_plane);
	}
	
	Vector3 GetForce(Vector3 const & target_position, Plane3 const & horizontal_plane, Scalar thrust)
	{
		CRAG_VERIFY_UNIT(horizontal_plane.normal, .00001f);
		
		auto const & horizontal_direction = geom::Clamped(target_position - horizontal_plane.position, ufo_controller2_max_direction);
		auto const & vertical_direction = horizontal_plane.normal * ufo_controller2_drag_roof_height;
		auto const & direction = geom::Normalized(horizontal_direction + vertical_direction);

		auto const & force = direction * thrust;
		
		return force;
	}
}

////////////////////////////////////////////////////////////////////////////////
// sim::UfoController2 member functions

UfoController2::UfoController2(Entity & entity, std::shared_ptr<Entity> const & ball_entity, Scalar thrust)
: Controller(entity)
, _ball_entity(ball_entity)
, _thrust(thrust)
{
	auto & frustum = _pov.GetFrustum();
	frustum.depth_range = { 1, 2 };
	
	auto & roster = entity.GetEngine().GetTickRoster();
	roster.AddCommand(* this, & UfoController2::Tick);

	CRAG_VERIFY(* this);
}

UfoController2::~UfoController2()
{
	CRAG_VERIFY(* this);
	
	auto & engine = GetEntity().GetEngine();

	// deactivate listener
	ipc::Listener<Engine, gfx::SetCameraEvent>::SetIsListening(false);
	
	// roster
	auto & roster = engine.GetTickRoster();
	roster.RemoveCommand(* this, & UfoController2::Tick);
	
	// remove ball
	if (_ball_entity)
	{
		engine.ReleaseObject(* _ball_entity);
	}
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(UfoController2, self)
	CRAG_VERIFY(self._pov.GetTransformation());
	if (self._ball_entity)
	{
		CRAG_VERIFY(* self._ball_entity);
		CRAG_VERIFY_TRUE(self._ball_entity->GetHandle().IsInitialized());
	}
CRAG_VERIFY_INVARIANTS_DEFINE_END

void UfoController2::operator() (gfx::SetCameraEvent const & event)
{
	auto const & space = GetEntity().GetEngine().GetSpace();

	_pov.SetTransformation(space.AbsToRel(event.transformation));
	_pov.GetFrustum().fov = event.fov;
}

void UfoController2::Tick()
{
	geom::Vector2i pixel_position;
	auto mouse_state = SDL_GetMouseState(& pixel_position.x, & pixel_position.y);
	if ((mouse_state & SDL_BUTTON(1)) == 0)
	{
		return;
	}
	
	auto const & location = GetEntity().GetLocation();
	if (! location)
	{
		return;
	}

	// get UFO position
	auto & body = core::StaticCast<physics::Body>(* location);
	auto const & transformation = body.GetTransformation();
	auto const & translation = transformation.GetTranslation();
	auto const & rotation = transformation.GetRotation();

	// horizontal plane
	auto const & up = GetUp(body.GetGravitationalForce());
	auto const & horizontal_plane = Plane3(translation, up);

	// target position
	Vector3 target_position = GetTargetPosition(_pov, pixel_position, horizontal_plane);
	
	// force
	auto const & force = GetForce(target_position, horizontal_plane, _thrust);
	
	// point at top of UFO
	auto const & ufo_up = gfx::GetAxis(rotation, gfx::Direction::forward);
	auto const & ufo_top = translation + ufo_up * saucer_ball_radius;

	body.AddForceAtPos(force, ufo_top);
}

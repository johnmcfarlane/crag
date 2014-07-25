//
//  CameraController.cpp
//  crag
//
//  Created by John on 2014-01-30.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "CameraController.h"

#include "sim/Engine.h"
#include "sim/Entity.h"
#include "sim/gravity.h"

#include "physics/RayCast.h"

#include "gfx/axes.h"
#include "gfx/SetCameraEvent.h"

#include "core/ConfigEntry.h"
#include "core/Roster.h"

#include "geom/origin.h"

using namespace sim;

namespace
{
	CONFIG_DEFINE (camera_controller_height, float, 7.5f);
	CONFIG_DEFINE (camera_controller_distance, float, 10.f);
	CONFIG_DEFINE (camera_push_magnitude, float, 1000.f);
	CONFIG_DEFINE (camera_lod_radius, float, 5.f);
	
	void UpdateLodParameters(Vector3 const & /*camera_translation*/, Vector3 const & subject_translation)
	{
		// broadcast new camera position
		gfx::SetLodParametersEvent event;
		event.parameters.center = subject_translation;
		event.parameters.min_distance = camera_lod_radius;
		Daemon::Broadcast(event);
	}

	// given information about camera location and direction, 
	// send a 'set camera' event
	void UpdateCamera(Vector3 const & camera_translation, geom::abs::Vector3 const & origin, Vector3 const & forward, Vector3 const & up)
	{
		auto dot = geom::DotProduct(forward, up);
		if (dot > .99999f || dot < -.99999f)
		{
			// subject is above or below camera; cannot calculate sensible camera up
			return;
		}

		auto rotation = gfx::Rotation(forward, up);

		// broadcast new camera position
		gfx::SetCameraEvent event;
		event.transformation.SetTranslation(geom::RelToAbs(camera_translation, origin));
		event.transformation.SetRotation(rotation);
		Daemon::Broadcast(event);
	}
	
	// nudge body forward and upward, based on distance from subject 
	// and altitude as measured by ray cast
	void UpdateBody(physics::Body & camera_body, physics::RayCast const & ray_cast, Vector3 const & forward, Vector3 const & up, float distance)
	{
		// apply linear forces to camera
		Vector3 push = Vector3::Zero();

		auto & ray_cast_result = ray_cast.GetResult();
		auto altitude = ray_cast_result.GetDistance();
		if (altitude < camera_controller_height)
		{
			push += up * (1.f - (altitude / camera_controller_height));
		}

		push += forward * (distance - camera_controller_distance) / camera_controller_distance;

		camera_body.AddForce(push * camera_push_magnitude);
	}
}

////////////////////////////////////////////////////////////////////////////////
// CameraController member definitions

CameraController::CameraController(Entity & entity, std::shared_ptr<Entity> const & subject)
: _super(entity)
, _ray_cast(* new physics::RayCast(entity.GetEngine().GetPhysicsEngine(), camera_controller_height))
, _subject(subject)
{
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.AddOrdering(& CameraController::Tick, & Entity::Tick);
	roster.AddCommand(* this, & CameraController::Tick);
}

CameraController::~CameraController()
{
	// roster
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.RemoveCommand(* this, & CameraController::Tick);

	delete & _ray_cast;
}

void CameraController::Tick()
{
	auto & camera_body = GetBody();
	auto camera_transformation = camera_body.GetTransformation();
	auto camera_translation = camera_transformation.GetTranslation();

	if (! _subject)
	{
		DEBUG_BREAK("camera has no subject");
		return;
	}
	auto const & subject = * _subject;
	
	auto subject_location = subject.GetLocation();
	if (! subject_location)
	{
		DEBUG_BREAK("camera's subject has no location");
		return;
	}
	
	auto const & subject_body = core::StaticCast<physics::Body const>(* subject_location);
	Vector3 up = GetUp(subject_body.GetGravitationalForce());
	if (up == Vector3::Zero())
	{
		return;
	}
	
	auto const & subject_translation = subject_location->GetTranslation();
	auto camera_to_subject = subject_translation - camera_translation;
	auto distance = geom::Length(camera_to_subject);
	
	if (distance < .01f)
	{
		// camera is too close to subject for relative direction to be much use
		return;
	}
	
	auto & engine = GetEntity().GetEngine();
	const auto & origin = engine.GetOrigin();
	auto forward = camera_to_subject / distance;

	UpdateLodParameters(camera_translation, subject_translation);
	UpdateCamera(camera_translation, origin, forward, up);
	UpdateBody(camera_body, _ray_cast, forward, up, distance);
	UpdateCameraRayCast();
}

void CameraController::UpdateCameraRayCast() const
{
	const auto & engine = GetEntity().GetEngine();
	
	auto & camera_body = GetBody();
	auto camera_transformation = camera_body.GetTransformation();
	auto camera_translation = camera_transformation.GetTranslation();

	const auto & origin = engine.GetOrigin();
	auto relative_origin = geom::AbsToRel(geom::Vector3d::Zero(), origin);
	auto up = geom::Normalized(camera_translation - relative_origin);

	_ray_cast.SetRay(Ray3(camera_translation, - up));
}

physics::Body & CameraController::GetBody()
{
	auto & entity = GetEntity();
	auto const & location = entity.GetLocation();
	return core::StaticCast<physics::Body>(* location);
}

physics::Body const & CameraController::GetBody() const
{
	auto & entity = GetEntity();
	auto const & location = entity.GetLocation();
	return core::StaticCast<physics::Body const>(* location);
}

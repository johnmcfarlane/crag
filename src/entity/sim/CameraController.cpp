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
#include "core/RosterObjectDefine.h"

#include "geom/Space.h"

using namespace sim;

CONFIG_DECLARE_ANGLE(frustum_default_fov, float);

namespace
{
	CONFIG_DEFINE(camera_controller_height, 7.5f);
	CONFIG_DEFINE(camera_controller_elevation, 2.5f);
	CONFIG_DEFINE(camera_controller_distance, 10.f);
	CONFIG_DEFINE(camera_push_magnitude, 100.f);
	CONFIG_DEFINE(camera_lod_radius, 2.5f);
	
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
	void UpdateCamera(Transformation const & camera_transformation, geom::Space const & space, Vector3 const & forward, Vector3 up)
	{
		using geom::abs::Scalar;

		// if subject is above or below camera,
		auto dot = geom::DotProduct(forward, up);
		if (dot > 1.f || dot < -1.f)
		{
			// cannot calculate sensible camera up; use previous value
			up = gfx::GetAxis(camera_transformation.GetRotation(), gfx::Direction::up);
		}

		gfx::SetCameraEvent event = {
			// calculate sensible camera up
			geom::Transformation<Scalar>(
				space.RelToAbs(camera_transformation.GetTranslation()),
				gfx::Rotation(
					geom::Cast<Scalar>(forward), 
					geom::Cast<Scalar>(up))),
			frustum_default_fov
		};

		Daemon::Broadcast(event);
	}
	
	// nudge body forward and upward, based on distance from subject 
	// and altitude as measured by ray cast
	void UpdateBody(
		physics::Body & camera_body, 
		physics::RayCast const & ray_cast, 
		Vector3 const & subject_translation, 
		Vector3 const & up)
	{
		auto const & camera_translation = camera_body.GetTranslation();
		Plane3 const subject_plane(subject_translation, up);
		auto elevation = geom::Distance(subject_plane, camera_translation);
		
		auto horizontal_camera = camera_translation - up * elevation;
		auto horizontal_to_camera = horizontal_camera - subject_translation;
		
		auto desired_elevation = camera_controller_elevation;

		auto & ray_cast_result = ray_cast.GetResult();
		if (ray_cast_result)
		{
			auto altitude = ray_cast_result.GetDistance();
			desired_elevation += camera_controller_height - altitude;
		}
			
		auto const & desired = subject_translation + geom::Resized(
			horizontal_to_camera,
			camera_controller_distance) + up * desired_elevation;
			
		auto const & push = desired - camera_translation;
		
		camera_body.AddForce(push * camera_push_magnitude);
	}
}

////////////////////////////////////////////////////////////////////////////////
// CameraController member definitions

CRAG_ROSTER_OBJECT_DEFINE(
	CameraController,
	1,
	Pool::CallBefore<& CameraController::Tick, Entity, & Entity::Tick>(Engine::GetTickRoster()))

CameraController::CameraController(Entity & entity, std::shared_ptr<Entity> const & subject)
: _super(entity)
, _ray_cast(new physics::RayCast(entity.GetEngine().GetPhysicsEngine(), camera_controller_height))
, _subject(subject)
{
}

void CameraController::Tick()
{
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
	
	auto & camera_body = GetBody();
	Vector3 up = GetUp(camera_body.GetGravitationalForce());
	if (up == Vector3::Zero())
	{
		return;
	}
	
	auto camera_transformation = camera_body.GetTransformation();
	auto camera_translation = camera_transformation.GetTranslation();

	auto const & subject_translation = subject_location->GetTranslation();
	auto camera_to_subject = subject_translation - camera_translation;
	auto distance = geom::Magnitude(camera_to_subject);
	
	if (distance < .01f)
	{
		// camera is too close to subject for relative direction to be much use
		return;
	}
	
	auto & engine = GetEntity().GetEngine();
	const auto & space = engine.GetSpace();
	auto forward = camera_to_subject / distance;

	UpdateLodParameters(camera_translation, subject_translation);
	UpdateCamera(camera_transformation, space, forward, up);
	UpdateBody(camera_body, * _ray_cast, subject_translation, up);
	UpdateCameraRayCast();
}

void CameraController::UpdateCameraRayCast() const
{
	auto & camera_body = GetBody();
	auto const & up = GetUp(camera_body.GetGravitationalForce());
	if (up == Vector3::Zero())
	{
		return;
	}
	
	auto camera_transformation = camera_body.GetTransformation();
	auto camera_translation = camera_transformation.GetTranslation();

	_ray_cast->SetRay(Ray3(camera_translation, - up));
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

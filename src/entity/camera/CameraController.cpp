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

#include "entity/animat/Sensor.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/Body.h"

#include "gfx/axes.h"
#include "gfx/SetCameraEvent.h"

#include "core/ConfigEntry.h"
#include "core/Roster.h"

namespace
{
	CONFIG_DEFINE (camera_controller_height, float, 10.f);
}

////////////////////////////////////////////////////////////////////////////////
// CameraController member definitions

using namespace sim;

CameraController::CameraController(Entity & entity, EntityHandle subject)
: _super(entity)
, _sensor(* new Sensor(entity, Ray3(Vector3::Zero(), Vector3(0.f, -1.f, 0.f)), camera_controller_height))
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

	delete & _sensor;
}

void CameraController::Tick()
{
	// send last location update to rendered etc.
	UpdateCamera();

	// event-based input
	HandleEvents();
}

void CameraController::HandleEvents()
{
	SDL_Event event;
	while (_event_watcher.PopEvent(event))
	{
		HandleEvent(event);
	}
}

void CameraController::HandleEvent(SDL_Event const & event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
			HandleKeyboardEvent(event.key.keysym.scancode, true);
			break;
		
		case SDL_KEYUP:
			HandleKeyboardEvent(event.key.keysym.scancode, false);
			break;
		
		default:
			break;
	}
}

// returns false if it's time to quit
void CameraController::HandleKeyboardEvent(SDL_Scancode scancode, bool down)
{
	if (! down)
	{
		return;
	}

	switch (scancode)
	{
		case SDL_SCANCODE_C:
			{
				_collidable = ! _collidable;
				GetBody().SetIsCollidable(_collidable);
			}
			break;
			
		default:
			break;
	}
}

void CameraController::UpdateCamera() const
{
	const auto & body = GetBody();
	auto translation = body.GetTranslation();
	auto transformation = Transformation(translation);

	const auto & engine = GetEntity().GetEngine();
	auto subject = engine.GetObject(_subject.GetUid());
	if (subject)
	{
		auto subject_location = subject->GetLocation();
		if (subject_location)
		{
			auto camera_to_subject = subject_location->GetTranslation() - translation;
			auto distance = geom::Length(camera_to_subject);
			
			auto calc_rotation = [&] () -> Matrix33
			{
				if (distance > 0.1)
				{
					auto forward = camera_to_subject / distance;
					auto up = gfx::GetAxis(subject_location->GetRotation(), gfx::Direction::up);
					auto dot = geom::DotProduct(forward, up);
					
					if (dot < .999f && dot > -.999f)
					{
						return geom::Inverse(gfx::Rotation(forward, up));
					}
				}

				return subject_location->GetRotation();
			};
			
			transformation.SetRotation(calc_rotation());
		}
	}
	
	const auto & origin = engine.GetOrigin();

	// broadcast new camera position
	gfx::SetCameraEvent event;
	event.transformation = geom::RelToAbs(transformation, origin);
	Daemon::Broadcast(event);
}

physics::Body & CameraController::GetBody()
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

physics::Body const & CameraController::GetBody() const
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

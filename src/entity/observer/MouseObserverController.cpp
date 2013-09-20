//
//  Controller.cpp
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "MouseObserverController.h"
#include "ObserverInput.h"

#include "entity/animat/Sensor.h"

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "physics/Body.h"

#include "gfx/SetCameraEvent.h"

#include "ipc/Daemon.h"

#include "geom/Matrix33.h"
#include "geom/Transformation.h"

#include "core/ConfigEntry.h"
#include "core/Roster.h"

CONFIG_DECLARE (sim_tick_duration, core::Time);

namespace
{
	CONFIG_DEFINE (observer_speed, int, 1);
	CONFIG_DEFINE (observer_translation_input, float, 10.08f);
	CONFIG_DEFINE (observer_rotation_input, float, 12.6f);
	CONFIG_DEFINE (observer_mouse_sensitivity, float, 0.35f);

	// TODO: this value is likely sensitive to screen resolutions
#if defined(__APPLE__)
	CONFIG_DEFINE (observer_mouse_sensitivity_platform_factor, float, 1.0f);
#elif defined(WIN32)
	CONFIG_DEFINE (observer_mouse_sensitivity_platform_factor, float, 0.5f);
#else
	CONFIG_DEFINE (observer_mouse_sensitivity_platform_factor, float, 0.1f);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// MouseObserverController member definitions

using namespace sim;

MouseObserverController::MouseObserverController(Entity & entity)
: _super(entity)
, _speed(observer_speed)
, _sensor(* new Sensor(entity, Ray3(Vector3::Zero(), Vector3(0.f, 0.f, 1.f)), 1000000.f))
{
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.AddOrdering(& MouseObserverController::Tick, & Entity::Tick);
	roster.AddCommand(* this, & MouseObserverController::Tick);
}

MouseObserverController::~MouseObserverController()
{
	// roster
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.RemoveCommand(* this, & MouseObserverController::Tick);

	// record speed in config file
	observer_speed = _speed;
	
	delete & _sensor;
}

void MouseObserverController::Tick()
{
	// send last location update to rendered etc.
	UpdateCamera();

	// state-based input
	ObserverInput input = GetObserverInput();

	// event-based input
	HandleEvents(input);

	ScaleInput(input);
	ApplyInput(input);
}

void MouseObserverController::HandleEvents(ObserverInput & input)
{
	SDL_Event event;
	while (_event_watcher.PopEvent(event))
	{
		HandleEvent(input, event);
	}
}

void MouseObserverController::HandleEvent(ObserverInput & input, SDL_Event const & event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
			HandleKeyboardEvent(event.key.keysym.scancode, 1);
			break;
		
		case SDL_KEYUP:
			HandleKeyboardEvent(event.key.keysym.scancode, 0);
			break;
		
		case SDL_MOUSEMOTION:
			HandleMouseMove(input, event.motion);
			break;
		
		default:
			break;
	}
}

// returns false if it's time to quit
void MouseObserverController::HandleKeyboardEvent(SDL_Scancode scancode, bool down)
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

		case SDL_SCANCODE_0:
			_speed = 10;
			break;
		
		default:
			if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
			{
				_speed = scancode + 1 - SDL_SCANCODE_1;
			}
			break;
	}
}

void MouseObserverController::HandleMouseMove(ObserverInput & input, SDL_MouseMotionEvent const & motion) const
{
	input[ObserverInput::rotation].x += motion.yrel * observer_mouse_sensitivity_platform_factor * observer_mouse_sensitivity;
	input[ObserverInput::rotation].y += motion.xrel * observer_mouse_sensitivity_platform_factor * observer_mouse_sensitivity;
}

void MouseObserverController::ScaleInput(ObserverInput & input) const
{
	Scalar dt = Scalar(sim_tick_duration);

	Scalar speed_factor = std::pow(std::pow(10.f, .4f), (_speed << 1) + 1);
	Scalar translation_factor = observer_translation_input * speed_factor * dt;
	input[ObserverInput::translation] *= translation_factor;

	Scalar rotation_factor = observer_rotation_input * dt;
	input[ObserverInput::rotation] *= rotation_factor;
}

void MouseObserverController::ApplyInput(ObserverInput const & input)
{
	auto& body = GetBody();
	body.AddRelForce(input [ObserverInput::translation]);
	body.AddRelTorque(input [ObserverInput::rotation]);
}

void MouseObserverController::UpdateCamera() const
{
	auto& body = GetBody();
	auto translation = body.GetTranslation();
	auto rotation = body.GetRotation();
	Transformation transformation (translation, rotation);

	// broadcast new camera position
	gfx::SetCameraEvent event;
	event.transformation = transformation;
	Daemon::Broadcast(event);
}

physics::Body & MouseObserverController::GetBody()
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

physics::Body const & MouseObserverController::GetBody() const
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

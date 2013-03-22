//
//  Controller.cpp
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ObserverController.h"

#include "axes.h"
#include "Engine.h"
#include "Entity.h"
#include "ObserverInput.h"

#include "physics/Body.h"

#include "gfx/Engine.h"

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
// ObserverController member definitions

using namespace sim;

ObserverController::ObserverController(Entity & entity)
: _super(entity)
, _speed(observer_speed)
{
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.AddOrdering(& ObserverController::Tick, & Entity::Tick);
	roster.AddCommand(* this, & ObserverController::Tick);
}

ObserverController::~ObserverController()
{
	// roster
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.RemoveCommand(* this, & ObserverController::Tick);

	// record speed in config file
	observer_speed = _speed;
}

void ObserverController::Tick()
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

void ObserverController::HandleEvents(ObserverInput & input)
{
	SDL_Event event;
	while (_event_watcher.PopEvent(event))
	{
		HandleEvent(input, event);
	}
}

void ObserverController::HandleEvent(ObserverInput & input, SDL_Event const & event)
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
void ObserverController::HandleKeyboardEvent(SDL_Scancode scancode, bool down)
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

void ObserverController::HandleMouseMove(ObserverInput & input, SDL_MouseMotionEvent const & motion) const
{
	input[ObserverInput::rotation].x -= motion.yrel * observer_mouse_sensitivity_platform_factor * observer_mouse_sensitivity;
	input[ObserverInput::rotation].z -= motion.xrel * observer_mouse_sensitivity_platform_factor * observer_mouse_sensitivity;
}

void ObserverController::ScaleInput(ObserverInput & input) const
{
	Scalar dt = Scalar(sim_tick_duration);

	Scalar speed_factor = std::pow(std::pow(10.f, .4f), (_speed << 1) + 1);
	Scalar translation_factor = observer_translation_input * speed_factor * dt;
	input[ObserverInput::translation] *= translation_factor;

	Scalar rotation_factor = observer_rotation_input * dt;
	input[ObserverInput::rotation] *= rotation_factor;
}

void ObserverController::ApplyInput(ObserverInput const & input)
{
	auto& body = GetBody();
	body.AddRelForce(input [ObserverInput::translation]);
	body.AddRelTorque(input [ObserverInput::rotation]);
}

void ObserverController::UpdateCamera() const
{
	auto& body = GetBody();
	auto position = body.GetPosition();
	auto rotation = body.GetRotation();
	Transformation transformation (position, rotation);
	auto camera_ray = axes::GetCameraRay(transformation);

	// update sim (sends message to gfx)
	auto& engine = GetEntity().GetEngine();
	engine.SetCamera(camera_ray);

	// update gfx
	// TODO: communicate this through the Entity's branch node
	gfx::Daemon::Call([transformation] (gfx::Engine & engine) {
		engine.OnSetCamera(transformation);
	});
}

physics::Body & ObserverController::GetBody()
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

physics::Body const & ObserverController::GetBody() const
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

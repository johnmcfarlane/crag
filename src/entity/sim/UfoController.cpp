//
//  entity/sim/UfoController.cpp
//  crag
//
//  Created by John McFarlane on 2014-03-20.
//	Copyright 2014 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "UfoController.h"
#include "RoverThruster.h"

#include "sim/Entity.h"
#include "sim/Engine.h"

#include "physics/Body.h"

#include "gfx/axes.h"

#include "core/ConfigEntry.h"
#include "core/Roster.h"

using namespace sim;

namespace
{
	CONFIG_DEFINE (ufo_controlled_thrust, physics::Scalar, 4.5f);
	
#if defined(CRAG_USE_MOUSE)
	CONFIG_DEFINE(ufo_controller_sensitivity, Scalar, 5.f);
#endif

#if defined(CRAG_USE_TOUCH)
	CONFIG_DEFINE(ufo_controller_sensitivity, Scalar, 100000.f);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// sim::UfoController member functions

UfoController::UfoController(Entity & entity)
: VehicleController(entity)
, _camera_right(1.f, 0.f, 0.f)
, _camera_forward(0.f, 0.f, 1.f)
, _main_thruster(new Thruster(entity, Ray3(Vector3(0.f, -.2f, 0.f), Vector3(0.f, ufo_controlled_thrust, 0.f)), false, 1.f))
, _num_presses(0)
{
	AddThruster(VehicleController::ThrusterPtr(_main_thruster));

	auto & roster = entity.GetEngine().GetTickRoster();
	roster.AddCommand(* this, & UfoController::Tick);

	CRAG_VERIFY(* this);
}

UfoController::~UfoController()
{
	ipc::Listener<Engine, gfx::SetCameraEvent>::SetIsListening(false);

	// roster
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.RemoveCommand(* this, & UfoController::Tick);
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(UfoController, self)
	CRAG_VERIFY(self._camera_right);
	CRAG_VERIFY(self._camera_forward);
CRAG_VERIFY_INVARIANTS_DEFINE_END

void UfoController::Tick()
{
	Vector2 pointer_delta = HandleEvents();

	ApplyThrust(pointer_delta);
	ApplyTilt(pointer_delta);
}

void UfoController::ApplyThrust(Vector2 pointer_delta)
{
	bool is_rotating = pointer_delta != Vector2::Zero();
	auto thrust_factor = ShouldThrust(is_rotating) ? 1.f : 0.f;
	_main_thruster->SetThrustFactor(thrust_factor);
}

bool UfoController::ShouldThrust(bool) const
{
	return _num_presses > 0;
}

void UfoController::ApplyTilt(Vector2 pointer_delta)
{
	if (pointer_delta == Vector2::Zero())
	{
		return;
	}

	auto location = GetEntity().GetLocation();
	if (! location)
	{
		return;
	}
	auto & body = core::StaticCast<physics::Body>(* location);
	
	auto resolution = app::GetResolution();
	Vector2 drag(
		ufo_controller_sensitivity * pointer_delta.x / resolution.x,
		ufo_controller_sensitivity * pointer_delta.y / resolution.y);
	
	auto ufo_down = geom::Normalized(body.GetGravitationalForce());

	Vector3 ufo_forward, ufo_right;
	if (geom::Length(ufo_down) > 0)
	{
		ufo_forward = geom::Normalized(geom::CrossProduct(ufo_down, _camera_right));
		ufo_right = geom::Normalized(geom::CrossProduct(ufo_forward, ufo_down));
	}
	else
	{
		ufo_forward = _camera_forward;
		ufo_right = _camera_right;
	}
	
	auto tilt = ufo_right * drag.x - ufo_forward * drag.y;
	body.AddForceAtPos(tilt, body.GetTranslation() - ufo_down);
}

Vector2 UfoController::HandleEvents()
{
	auto pointer_delta = Vector2::Zero();
	
	SDL_Event event;
	while (_event_watcher.PopEvent(event))
	{
		pointer_delta += HandleEvent(event);
	}
	
	return pointer_delta;
}

Vector2 UfoController::HandleEvent(SDL_Event const & event)
{
	switch (event.type)
	{
#if defined(CRAG_USE_MOUSE)
		case SDL_KEYDOWN:
			if (! event.key.repeat)
			{
				HandleKeyboardEvent(event.key.keysym.scancode, true);
			}
			break;
		
		case SDL_KEYUP:
			HandleKeyboardEvent(event.key.keysym.scancode, false);
			break;
		
		case SDL_MOUSEMOTION:
			return Vector2(event.motion.xrel, event.motion.yrel);
			
		case SDL_MOUSEBUTTONDOWN:
			++ _num_presses;
			ASSERT(_num_presses > 0);
			break;
		
		case SDL_MOUSEBUTTONUP:
			ASSERT(_num_presses > 0);
			-- _num_presses;
			break;
#endif

#if defined(CRAG_USE_TOUCH)
		case SDL_FINGERMOTION:
			return Vector2(event.tfinger.dx, event.tfinger.dy);
			
		case SDL_FINGERDOWN:
			++ _num_presses;
			ASSERT(_num_presses > 0);
			break;
			
		case SDL_FINGERUP:
			ASSERT(_num_presses > 0);
			-- _num_presses;
			break;
#endif
			
		default:
			break;
	}
	
	return Vector2::Zero();
}

void UfoController::HandleKeyboardEvent(SDL_Scancode scancode, bool down)
{
	switch (scancode)
	{
		case SDL_SCANCODE_SPACE:
			if (down)
			{
				++ _num_presses;
			}
			else
			{
				-- _num_presses;
			}
			break;
		
		default:
			break;
	}
}

void UfoController::operator() (gfx::SetCameraEvent const & event)
{
	auto const & rotation = event.transformation.GetRotation();
	_camera_right = geom::Cast<Scalar>(gfx::GetAxis(rotation, gfx::Direction::right));
	_camera_forward = geom::Cast<Scalar>(gfx::GetAxis(rotation, gfx::Direction::forward));
}

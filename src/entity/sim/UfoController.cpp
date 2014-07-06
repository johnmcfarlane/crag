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
#if defined(CRAG_USE_MOUSE)
	CONFIG_DEFINE(ufo_controller_sensitivity, Scalar, 100.f);
#endif

#if defined(CRAG_USE_TOUCH)
	CONFIG_DEFINE(ufo_controller_sensitivity, Scalar, 2000000.f);
#endif
}

////////////////////////////////////////////////////////////////////////////////
// sim::UfoController member functions

UfoController::UfoController(Entity & entity, EntityHandle ball_entity, Scalar max_thrust)
: VehicleController(entity)
, _camera_rotation(Matrix33::Identity())
, _ball_entity(ball_entity)
, _main_thruster(new Thruster(entity, Ray3(Vector3(0.f, 0.f, -.2f), Vector3(0.f, 0.f, max_thrust)), false, 1.f))
, _num_presses(0)
{
	AddThruster(VehicleController::ThrusterPtr(_main_thruster));

	auto & roster = entity.GetEngine().GetTickRoster();
	roster.AddCommand(* this, & UfoController::Tick);

	CRAG_VERIFY(* this);
}

UfoController::~UfoController()
{
	auto & engine = GetEntity().GetEngine();

	// deactivate listener
	ipc::Listener<Engine, gfx::SetCameraEvent>::SetIsListening(false);
	
	// roster
	auto & roster = engine.GetTickRoster();
	roster.RemoveCommand(* this, & UfoController::Tick);
	
	// remove ball
	if (_ball_entity)
	{
		engine.DestroyObject(_ball_entity.GetUid());
	}
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(UfoController, self)
	CRAG_VERIFY(self._camera_rotation);
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

// returns true iff UFO is upside down
void UfoController::ApplyTilt(Vector2 pointer_delta)
{
	auto location = GetEntity().GetLocation();
	if (! location)
	{
		return;
	}
	
	auto & body = core::StaticCast<physics::Body>(* location);
	
	auto resolution = app::GetResolution();
	auto factor = ufo_controller_sensitivity / geom::Length(geom::Cast<float>(resolution));
	Vector2 drag(pointer_delta.x * factor, pointer_delta.y * factor);
	
	auto touch_pad_right = gfx::GetAxis(_camera_rotation, gfx::Direction::right);
	auto touch_pad_up = gfx::GetAxis(_camera_rotation, gfx::Direction::forward);
	auto touch_pad_normal = gfx::GetAxis(_camera_rotation, gfx::Direction::up);

	auto tilt = touch_pad_right * drag.x + touch_pad_up * - drag.y;
	auto translation = body.GetTranslation();
	
	body.AddForceAtPos(tilt, translation + touch_pad_normal);
	body.AddForceAtPos(- tilt, translation - touch_pad_normal);
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
			return Vector2(Scalar(event.motion.xrel), Scalar(event.motion.yrel));
			
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
	_camera_rotation = geom::Cast<Scalar>(event.transformation.GetRotation());
}

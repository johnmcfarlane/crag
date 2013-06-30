//
//  TouchObserverController.cpp
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "TouchObserverController.h"

#include "Engine.h"
#include "Entity.h"

#include "physics/Body.h"

#include "gfx/axes.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetOriginEvent.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/Roster.h"

#include "geom/Quaternion.h"

extern float camera_fov;

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// TouchObserverController member definitions

struct TouchObserverController::Finger
{
	// camera transformation at point where finger pressed down
	Transformation down_transformation;
	
	// pixel direction where finger pressed down
	Vector3 down_direction;
	
	// pixel direction now
	Vector3 direction;
	
	// SDL ID of finger
	SDL_FingerID id;
};

TouchObserverController::TouchObserverController(Entity & entity, Transformation const & transformation)
: Controller(entity)
, _transformation(transformation)
, _origin(geom::abs::Vector3::Zero())
{
	_frustum.resolution = app::GetResolution();
	_frustum.depth_range = Vector2(1, 2);
	_frustum.fov = camera_fov;

	// register 
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.AddOrdering(& TouchObserverController::Tick, & Entity::Tick);
	roster.AddCommand(* this, & TouchObserverController::Tick);
}

TouchObserverController::~TouchObserverController()
{
	SetIsListening(false);

	// roster
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.RemoveCommand(* this, & TouchObserverController::Tick);
}

void TouchObserverController::Tick()
{
	// event-based input
	HandleEvents();
}

void TouchObserverController::operator() (gfx::SetOriginEvent const & event)
{
	auto previous_relative_camera_position = _transformation.GetTranslation();
	auto absolute_camera_pos = geom::RelToAbs(previous_relative_camera_position, _origin);

	_origin = event.origin;

	auto new_relative_camera_position = geom::AbsToRel(absolute_camera_pos, _origin);
	_transformation.SetTranslation(new_relative_camera_position);
}

void TouchObserverController::HandleEvents()
{
	SDL_Event event;
	while (_event_watcher.PopEvent(event))
	{
		HandleEvent(event);
	}
}

void TouchObserverController::HandleEvent(SDL_Event const & event)
{
	switch (event.type)
	{
#if defined(CRAG_USE_TOUCH)
		case SDL_FINGERDOWN:
		{
			auto direction = GetPixelDirection(GetScreenPosition(event.tfinger), _transformation);
			HandleFingerDown(direction, event.tfinger.fingerId);
			break;
		}
			
		case SDL_FINGERUP:
		{
			HandleFingerUp(event.tfinger.fingerId);
			ASSERT(_fingers.empty());
			break;
		}
			
		case SDL_FINGERMOTION:
		{
			if (_fingers.empty())
			{
				break;
			}

			auto direction = GetPixelDirection(GetScreenPosition(event.tfinger), GetDownTransformation());
			HandleFingerMotion(direction, event.tfinger.fingerId);
			break;
		}
#endif
		
#if defined(CRAG_USE_MOUSE)
		case SDL_MOUSEBUTTONDOWN:
		{
			auto direction = GetPixelDirection(GetScreenPosition(event.button), _transformation);
			SDL_FingerID finger_id(event.button.button);
			if (event.button.button == SDL_BUTTON_RIGHT && IsDown(finger_id))
			{
				HandleFingerUp(finger_id);
			}
			else
			{
				HandleFingerDown(direction, finger_id);
			}
			break;
		}
		
		case SDL_MOUSEBUTTONUP:
		{
			if (event.button.button != SDL_BUTTON_RIGHT)
			{
				HandleFingerUp(SDL_FingerID(event.button.button));
			}
			break;
		}
		
		case SDL_MOUSEMOTION:
		{
			// if the mouse button isn't pressed,
			if (! IsDown(SDL_FingerID(event.button.button)))
			{
				// ignore motion because we're emulating a touch device
				break;
			}
			
			auto direction = GetPixelDirection(GetScreenPosition(event.motion), GetDownTransformation());
			HandleFingerMotion(direction, SDL_FingerID(SDL_BUTTON_LEFT));
			break;
		}
#endif
		
		default:
			break;
	}
}

void TouchObserverController::HandleFingerDown(Vector3 const & direction, SDL_FingerID id)
{
	auto found = FindFinger(id);

	auto& finger = [&] () -> Finger&
	{
		if (found == _fingers.end())
		{
			_fingers.emplace_back();
			auto& finger = _fingers.back();
			finger.id = id;
			return finger;
		}
		else
		{
			DEBUG_MESSAGE("double-down detected");
			return * found;
		}
	} ();
	
	finger.down_transformation = _transformation;
	finger.down_direction = direction;
	finger.direction = direction;
	
	ASSERT(IsDown(id));
}

void TouchObserverController::HandleFingerUp(SDL_FingerID id)
{
	auto found = FindFinger(id);
	if (found == _fingers.end())
	{
		DEBUG_MESSAGE("double-up detected");
	}
	else
	{
		_fingers.erase(found);
	}
	
	ASSERT(! IsDown(id));
}

void TouchObserverController::HandleFingerMotion(Vector3 const & direction, SDL_FingerID id)
{
	auto found = FindFinger(id);
	if (found == _fingers.end())
	{
		DEBUG_MESSAGE("missing down");
		HandleFingerDown(direction, id);
		return;
	}
	
	found->direction = direction;
	UpdateCamera();
}

void TouchObserverController::UpdateCamera()
{
	switch (_fingers.size())
	{
		case 0:
			DEBUG_MESSAGE("erroneous cal to UpdateCamera");
			break;
			
		case 1:
			UpdateCameraRotation(_fingers.front());
			break;
			
		default:
			UpdateCameraTranslation(_fingers.front(), _fingers.back());
			break;
	}
}

void TouchObserverController::UpdateCameraRotation(Finger const & finger)
{
	// extract data about initial camera transformation
	auto & down_transformation = finger.down_transformation;
	Vector3 camera_position = down_transformation.GetTranslation();
	Matrix33 camera_rotation = down_transformation.GetRotation();
	Vector3 camera_up = gfx::GetAxis(camera_rotation, gfx::Direction::up);
	
	// get 'from' and 'to' finger pointing directions
	Vector3 relative_down_direction = geom::Normalized(finger.down_direction);
	Vector3 relative_current_direction = geom::Normalized(finger.direction);
	
	// convert them into rotations and take the difference
	auto from_matrix = gfx::Rotation(relative_down_direction, camera_up);
	auto to_matrix = gfx::Rotation(relative_current_direction, camera_up);
	auto required_rotation = Inverse(from_matrix) * (to_matrix);
	
	// apply them to camera rotation
	// TODO: Is operator* the right way around?
	camera_rotation = required_rotation * camera_rotation;

	// apply new camera rotation
	Transformation camera_transformation(camera_position, Matrix33(camera_rotation));
	SetTransformation(geom::Cast<float>(camera_transformation));
}

void TouchObserverController::SetTransformation(Transformation const & transformation)
{
	_transformation = transformation;

	// broadcast new camera position
	gfx::SetCameraEvent event;
	event.transformation = transformation;
	Daemon::Broadcast(event);
}

void TouchObserverController::UpdateCameraTranslation(Finger const & /*finger1*/, Finger const & /*finger2*/)
{
	ASSERT(false);
}

physics::Body & TouchObserverController::GetBody()
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

physics::Body const & TouchObserverController::GetBody() const
{
	auto& entity = GetEntity();
	auto& location = ref(entity.GetLocation());
	return ref(location.GetBody());
}

TouchObserverController::FingerVector::iterator TouchObserverController::FindFinger(SDL_FingerID id)
{
	return std::find_if(_fingers.begin(), _fingers.end(), [id] (Finger const & finger) 
	{
		return finger.id == id;
	});
}

TouchObserverController::FingerVector::const_iterator TouchObserverController::FindFinger(SDL_FingerID id) const
{
	return const_cast<TouchObserverController&>(*this).FindFinger(id);
}

bool TouchObserverController::IsDown(SDL_FingerID id) const
{
	return FindFinger(id) != _fingers.end();
}

Transformation const & TouchObserverController::GetDownTransformation() const
{
	if (_fingers.empty())
	{
		DEBUG_BREAK("Shouldn't even be here");
		return _transformation;
	}

	auto & start_finger = _fingers.front();
	auto & start_transformation = start_finger.down_transformation;
	return start_transformation;
}

Vector2 TouchObserverController::GetScreenPosition(SDL_TouchFingerEvent const & touch_finger_event) const
{
	Vector2 screen_position(2.f * touch_finger_event.x - 1.f, 1.f - 2.f * touch_finger_event.y);
	return screen_position;
}

Vector2 TouchObserverController::GetScreenPosition(SDL_MouseButtonEvent const & mouse_button_event) const
{
	Vector2 pixel_position(mouse_button_event.x, mouse_button_event.y);
	Vector2 screen_position = _frustum.PixelToScreen(pixel_position);
	return screen_position;
}

Vector2 TouchObserverController::GetScreenPosition(SDL_MouseMotionEvent const & mouse_motion_event) const
{
	Vector2 pixel_position(mouse_motion_event.x, mouse_motion_event.y);
	Vector2 screen_position = _frustum.PixelToScreen(pixel_position);
	return screen_position;
}

Vector3 TouchObserverController::GetPixelDirection(Vector2 const & screen_position, Transformation const & transformation) const
{
	gfx::Pov pov;
	pov.SetFrustum(_frustum);
	pov.SetTransformation(transformation);

	Vector3 position = pov.ScreenToWorld(screen_position);
	Vector3 direction = position - transformation.GetTranslation();

	return direction;
}

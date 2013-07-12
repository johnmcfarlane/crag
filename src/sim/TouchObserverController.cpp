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

namespace
{
	struct TranslationRollFinger
	{
		Vector3 const & world_position;
		Vector2 const & screen_position;
	};
	
	Transformation CalculateCameraTranslationRoll(TranslationRollFinger const & /*finger1*/, TranslationRollFinger const & /*finger2*/, Transformation const & camera_transformation)
	{
		//Vector3 const & forward = gfx::GetAxis(camera_transformation.GetRotation(), gfx::Direction::forward);
		return camera_transformation;
	}
}

////////////////////////////////////////////////////////////////////////////////
// TouchObserverController member definitions

struct TouchObserverController::Finger
{
	// camera transformation at point where finger pressed down
	Transformation down_transformation;
	
	// initial direction from camera to point of contact between finger and screen
	Vector3 world_position;
	
	// current direction from camera to point of contact between finger and screen
	Vector3 direction;
	
	// current screen position
	Vector2 screen_position;
	
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
			auto screen_position = GetScreenPosition(event.tfinger);
			HandleFingerDown(screen_position, event.tfinger.fingerId);
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

			auto screen_position = GetScreenPosition(event.tfinger);
			HandleFingerMotion(screen_position, event.tfinger.fingerId);
			break;
		}
#endif
		
#if defined(CRAG_USE_MOUSE)
		case SDL_MOUSEBUTTONDOWN:
		{
			SDL_FingerID finger_id(event.button.button);
			if (event.button.button == SDL_BUTTON_RIGHT && IsDown(finger_id))
			{
				HandleFingerUp(finger_id);
			}
			else
			{
				auto screen_position = GetScreenPosition(event.button);
				HandleFingerDown(screen_position, finger_id);
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
			
			auto screen_position = GetScreenPosition(event.motion);
			HandleFingerMotion(screen_position, SDL_FingerID(SDL_BUTTON_LEFT));
			break;
		}
#endif
		
		default:
			break;
	}
}

void TouchObserverController::HandleFingerDown(Vector2 const & screen_position, SDL_FingerID id)
{
	// find existing / create new Finger in _fingers and return reference
	auto& finger = [&] () -> Finger&
	{
		auto found = FindFinger(id);

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
	finger.direction = GetPixelDirection(screen_position, _transformation);
	finger.world_position = _transformation.GetTranslation() + finger.direction * 10.f;
	finger.screen_position = screen_position;
	
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

void TouchObserverController::HandleFingerMotion(Vector2 const & screen_position, SDL_FingerID id)
{
	auto found = FindFinger(id);
	if (found == _fingers.end())
	{
		DEBUG_MESSAGE("missing down");
		HandleFingerDown(screen_position, id);
		return;
	}
	
	found->direction = GetPixelDirection(screen_position, GetDownTransformation());
	found->screen_position = screen_position;
	
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
			UpdateCamera(_fingers.front());
			break;
			
		default:
			UpdateCamera(_fingers.front(), _fingers.back());
			break;
	}
}

// updates camera when one finger is down; 
// rotates the camera around a fixed position with fixed up axis 
// when one finger is touching the screen; 
// is intended to feel analogous to scrolling within a UI view
void TouchObserverController::UpdateCamera(Finger const & finger)
{
	// extract data about initial camera transformation
	auto & down_transformation = finger.down_transformation;
	Vector3 camera_position = down_transformation.GetTranslation();
	Matrix33 camera_rotation = down_transformation.GetRotation();
	Vector3 camera_up = gfx::GetAxis(camera_rotation, gfx::Direction::up);
	
	// get 'from' and 'to' finger pointing directions
	Vector3 relative_down_direction = geom::Normalized(finger.world_position - camera_position);
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

// updates camera when two fingers are down; 
// zooms/pans/rolls the camera when one finer is touching the screen;
// is intended to feel analogous to zooming and rotating a UI view:
// - moving fingers in parallel pans camera
// - rotating fingers rotated the camera rolls the camera / rotates along the z
// - moving fingers together/appart translates the camera along the z
void TouchObserverController::UpdateCamera(Finger const & finger1, Finger const & finger2)
{
	TranslationRollFinger translationRollFinger1 = 
	{
		finger1.world_position,
		finger1.screen_position
	};

	TranslationRollFinger translationRollFinger2 = 
	{
		finger2.world_position,
		finger2.screen_position
	};

	// the math
	auto camera_transformation = CalculateCameraTranslationRoll(translationRollFinger1, translationRollFinger2, _transformation);
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

// TODO: this is guesswork currently
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
	auto pixel_position = geom::MakeVector(mouse_button_event.x, mouse_button_event.y);
	auto screen_position = _frustum.PixelToScreen(geom::Cast<float>(pixel_position));
	return screen_position;
}

Vector2 TouchObserverController::GetScreenPosition(SDL_MouseMotionEvent const & mouse_motion_event) const
{
	auto pixel_position = geom::MakeVector(mouse_motion_event.x, mouse_motion_event.y);
	auto screen_position = _frustum.PixelToScreen(geom::Cast<float>(pixel_position));
	return screen_position;
}

// given pixel coordinates and the camera transformation,
// returns the vector from the camera to the screen position in world space
Vector3 TouchObserverController::GetPixelDirection(Vector2 const & screen_position, Transformation const & transformation) const
{
	gfx::Pov pov;
	pov.SetFrustum(_frustum);
	pov.SetTransformation(transformation);

	// position of the pixel on the made-up screen in world space
	Vector3 position = pov.ScreenToWorld(screen_position);
	Vector3 direction = position - transformation.GetTranslation();

	return direction;
}

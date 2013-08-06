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
#include "core/Random.h"
#include "core/Roster.h"

#include "geom/Quaternion.h"

#if ! defined(NDEBUG)
//#define SPAWN_SHAPES
#endif

#if defined(SPAWN_SHAPES)
#include "scripts/SpawnEntityFunctions.h"
#include "gfx/Color.h"
#endif

extern float camera_fov;

using namespace sim;

namespace
{
	struct TranslationRollFinger
	{
		Vector3 const & world_position;
		
		// vector from camera to touch-point on screen, so no shorter than unit length
		Vector3 const & screen_direction;
	};
	
	// http://en.wikipedia.org/wiki/Solution_of_triangles#Three_sides_given_.28SSS.29
	template <typename S>
	S SolveTriangleSSS(S a, S b, S c)
	{
		return std::acos((Squared(b) + Squared(c) - Squared(a)) / (2.f * b * c));
	}
	
	// returns results for a; assumes gamma is acute
	// http://en.wikipedia.org/wiki/Solution_of_triangles#Two_sides_and_non-included_angle_given_.28SSA.29
	template <typename S>
	S SolveTriangleSSA(S b, S c, S beta, bool second)
	{
		// verify input
		ASSERT(b == b);
		ASSERT(c == c);
		ASSERT(beta == beta);
		
		S c_over_b = c / b;
		S sin_beta = std::sin(beta);
		S sin_gamma = c_over_b * sin_beta;
		
		if (sin_gamma > 1)
		{
			// hopefully a precision error
			DEBUG_MESSAGE("Inpossible triangle. sin_gamma = %lf", double(sin_gamma));
			
			sin_gamma = 1.f;
		}
		
		S gamma = std::asin(sin_gamma);
		
		// if b < c
		if (std::fabs(c_over_b) > 1)
		{
			ASSERT(std::fabs(b) < std::fabs(c));
			
			// TODO: It may be possible for gamma to be obtuse intentionally.
			if (second)
			{
				gamma = float(PI) - gamma;
			}
		}
		else
		{
			ASSERT(std::fabs(b) >= std::fabs(c));
			ASSERT(gamma <= float(PI * .5));
		}
		
		S alpha = float(PI) - beta - gamma;
		S a = b * sin(alpha) / sin_beta;
		
		return a;
	}
	
	Transformation CalculateCameraTranslationRoll(TranslationRollFinger const & finger1, TranslationRollFinger const & finger2, Transformation const & camera_transformation)
	{
		const auto camera_rotation = camera_transformation.GetRotation();
		const auto camera_forward = gfx::GetAxis(camera_rotation, gfx::Direction::forward);
		
		/*//////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////
		// triangle height

		O = camera
		P = finger1.screen_position
		Q = finger2.screen_position
		A = finger1.world_position
		B = finger2.world_position
		The problem is that world is in different space to screen and camera
		although P rests on the line between A and O (and Q and B have a similar
		relationship).
		
		          a
		 C*-------M-------*B  ^ ^
		   \b            /    | |
		   A*-----------/     d |
			 \         /        |
			  \       /         h
			   \     /          |
		     ---*---*---      ^ |
		   ^   P \ / Q        1 |
		   |      *           v v
		forward   O                                                                   */
		
		////////////////////////////////////////////////////////////////////////////////
		// screen

		// vector from world_position #1 to world_position #2, AB
		const auto screen_position_diff = finger2.screen_direction - finger1.screen_direction;
		
		// distance between screen positions, PQ
		const auto screen_span = geom::Length(screen_position_diff);
		
		const auto screen_position_direction = screen_position_diff * 1.f / screen_span;
		
		const auto camera_to_screen_distance1 = geom::Length(finger1.screen_direction);
		const auto camera_to_screen_distance2 = geom::Length(finger2.screen_direction);
		
		const auto camera_to_screen_direction1 = finger1.screen_direction * 1.f / camera_to_screen_distance1;
		const auto camera_to_screen_direction2 = finger2.screen_direction * 1.f / camera_to_screen_distance2;
		
		////////////////////////////////////////////////////////////////////////////////
		// world

		// vector from world_position #1 to world_position #2, AB
		const auto world_position_diff = finger2.world_position - finger1.world_position;

		// how far forwards of world position #1 is #2, d
		const auto world_position_diff_forward = geom::DotProduct(world_position_diff, camera_forward);

		// distance between world positions, |AB| aka c
		const auto world_span = geom::Length(world_position_diff);
		
		const auto world_position_diff_direction = world_position_diff / world_span;

		// the distance we would extent OA to make it in line with B, |AC| aka b
		const auto equalizer = world_position_diff_forward * camera_to_screen_distance1;
		
		// cos(OPQ) - which is equal to cos(ACB)
		const auto cos_gamma = geom::DotProduct(screen_position_direction, - camera_to_screen_direction1);
		
		// triangle_width, a
		float triangle_width;
		
		// if world positions are just-about aligned,
		if (std::fabs(equalizer) * 1000 < world_span)
		{
			// then forego the precision errors and act like they really are
			triangle_width = world_span;
		}
		else
		{
			// else solve the triangle
			triangle_width = SolveTriangleSSA(world_span, equalizer, std::acos(cos_gamma), cos_gamma < 0);
		}
		
		// how far ahead of the camera is world position 2
		const auto world_position_forward2 = triangle_width / screen_span;
		
		// world position#2 in screen space
		const auto ss_world_position2 = camera_to_screen_direction2 * world_position_forward2 * camera_to_screen_distance2;
		const auto camera_to_world_distance2 = geom::Length(ss_world_position2);
		
		const auto get_angle = [] (Vector3 const & v, Matrix33 const & r)
		{
			Vector2 c(
				geom::DotProduct(v, gfx::GetAxis(r, gfx::Direction::right)),
				geom::DotProduct(v, gfx::GetAxis(r, gfx::Direction::up)));
			return std::atan2(c.x, c.y);
		};
		
		const auto rotation_angle_to = get_angle(world_position_diff_direction, camera_rotation);
		const auto rotation_angle_from = get_angle(screen_position_direction, camera_rotation);
		const auto rotation_angle = rotation_angle_to - rotation_angle_from;
		
		const auto output_rotation = camera_rotation * geom::Inverse(gfx::Rotation(gfx::Direction::forward, rotation_angle));
		
		auto ws_camera_to_screen_direction2 = ((camera_to_screen_direction2 * camera_rotation) * gfx::Rotation(gfx::Direction::forward, rotation_angle) * geom::Inverse(camera_rotation));
		const auto output_translation = finger2.world_position - ws_camera_to_screen_direction2 * camera_to_world_distance2;

		return Transformation(output_translation, output_rotation);
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

TouchObserverController::TouchObserverController(Entity & entity)
: Controller(entity)
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

	BroadcastTransformation();
}

void TouchObserverController::operator() (gfx::SetOriginEvent const & event)
{
	auto transformation = GetTransformation();
	auto previous_relative_camera_position = transformation.GetTranslation();
	auto absolute_camera_pos = geom::RelToAbs(previous_relative_camera_position, _origin);

	_origin = event.origin;

	auto new_relative_camera_position = geom::AbsToRel(absolute_camera_pos, _origin);
	transformation.SetTranslation(new_relative_camera_position);
	SetTransformation(transformation);
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
			DEBUG_MESSAGE("SDL_FINGERDOWN %d", int(event.tfinger.fingerId));
			auto screen_position = GetScreenPosition(event.tfinger);
			HandleFingerDown(screen_position, event.tfinger.fingerId);
			break;
		}
			
		case SDL_FINGERUP:
		{
			DEBUG_MESSAGE("SDL_UP %d", int(event.tfinger.fingerId));
			HandleFingerUp(event.tfinger.fingerId);
			//ASSERT(_fingers.empty());
			break;
		}
			
		case SDL_FINGERMOTION:
		{
			if (_fingers.empty())
			{
				DEBUG_MESSAGE("phantom motion %d", int(event.tfinger.fingerId));
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
	auto transformation = GetTransformation();

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
	
	finger.down_transformation = transformation;
	finger.direction = GetPixelDirection(screen_position, finger.down_transformation);
	
	auto random_distance = 5.f /*+ Random::sequence.GetUnit<float>() * 5.f*/;
	finger.world_position = finger.down_transformation.GetTranslation() + finger.direction * random_distance;
	finger.screen_position = screen_position;
	
#if defined(SPAWN_SHAPES)
	auto speed = 0.f;
	auto velocity = finger.direction * speed;
	switch (_fingers.size())
	{
		case 1:
			SpawnBall(Sphere3(finger.world_position, 0.5f), velocity, gfx::Color4f::Red());
			break;
			
		case 2:
			SpawnBox(finger.world_position, velocity, Vector3(1, 1, 1), gfx::Color4f::Blue());
			break;
	}
#endif
		
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
		if (_fingers.empty())
		{
			DEBUG_BREAK("missing down event");
			return;
		}
		ASSERT(_fingers.size() == 1);
		
		found = _fingers.begin();
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
			UpdateCamera(_fingers[0]);
			break;
			
		default:
			UpdateCamera(_fingers[0], _fingers[1]);
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
		finger1.direction
	};

	TranslationRollFinger translationRollFinger2 = 
	{
		finger2.world_position,
		finger2.direction
	};
	
	if (finger1.direction == finger2.direction)
	{
		DEBUG_MESSAGE("finger collision detected!");
		return;
	}

	// the math
	auto camera_transformation = CalculateCameraTranslationRoll(translationRollFinger1, translationRollFinger2, finger1.down_transformation);
	SetTransformation(geom::Cast<float>(camera_transformation));
}

Transformation const & TouchObserverController::GetTransformation() const
{
	auto & entity = GetEntity();
	auto location = entity.GetLocation();
	ASSERT(location);
	
	return location->GetTransformation();
}

void TouchObserverController::SetTransformation(Transformation const & transformation)
{
	auto & entity = GetEntity();
	auto location = entity.GetLocation();
	location->SetTransformation(transformation);
}

void TouchObserverController::BroadcastTransformation() const
{
	// broadcast new camera position
	gfx::SetCameraEvent event;
	event.transformation = GetTransformation();
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
		return GetTransformation();
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

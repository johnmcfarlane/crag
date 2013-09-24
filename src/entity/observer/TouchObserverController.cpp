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

#include "sim/Engine.h"
#include "sim/Entity.h"

#include "form/RayCastResult.h"

#include "physics/Body.h"
#include "physics/Engine.h"

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
	CONFIG_DEFINE (max_touch_ray_cast_distance, physics::Scalar, 1000000000.f);

	struct TranslationRollFinger
	{
		Vector3 const & world_position;
		
		// vector from camera to touch-point on screen, so no shorter than unit length
		Vector3 screen_position;
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
				gamma = S(PI) - gamma;
			}
		}
		else
		{
			VerifyOp(std::fabs(b), >=, std::fabs(c));
			VerifyOp(gamma, <=, S(PI * .5));
		}
		
		S alpha = S(PI) - beta - gamma;
		S a = b * sin(alpha) / sin_beta;
		
		return a;
	}
	
	Transformation CalculateCameraTranslationRoll(TranslationRollFinger const & finger1, TranslationRollFinger const & finger2, Transformation const & camera_transformation)
	{
		typedef float Scalar;
		typedef geom::Vector<Scalar, 2> Vector2;
		typedef geom::Vector<Scalar, 3> Vector3;
		typedef geom::Matrix<Scalar, 3, 3> Matrix33;
		
		const auto camera_rotation = geom::Cast<Scalar>(camera_transformation.GetRotation());
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
		const auto screen_position_diff = geom::Cast<Scalar>(finger2.screen_position) - geom::Cast<Scalar>(finger1.screen_position);
		
		// distance between screen positions, PQ
		const auto screen_span = geom::Length(screen_position_diff);
		
		const auto screen_position_direction = screen_position_diff * Scalar(1) / screen_span;
		if (screen_span < .00001f)
		{
			return camera_transformation;
		}
		
		const auto camera_to_screen_distance1 = geom::Length(geom::Cast<Scalar>(finger1.screen_position));
		const auto camera_to_screen_distance2 = geom::Length(geom::Cast<Scalar>(finger2.screen_position));
		
		const auto camera_to_screen_direction1 = geom::Cast<Scalar>(finger1.screen_position) * Scalar(1) / camera_to_screen_distance1;
		const auto camera_to_screen_direction2 = geom::Cast<Scalar>(finger2.screen_position) * Scalar(1) / camera_to_screen_distance2;
		
		////////////////////////////////////////////////////////////////////////////////
		// world

		// vector from world_position #1 to world_position #2, AB
		const auto world_position_diff = geom::Cast<Scalar>(finger2.world_position) - geom::Cast<Scalar>(finger1.world_position);

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
		Scalar triangle_width;
		
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
		/*const*/ auto camera_to_world_distance2 = world_position_forward2 * camera_to_screen_distance2;	// correct

		// how far ahead of the camera is world position 1
		const auto world_position_forward1 = world_position_forward2 - world_position_diff_forward;
		const auto camera_to_world_distance1 = world_position_forward1 * camera_to_screen_distance1;
		
		const auto get_angle = [] (Vector3 const & v, Matrix33 const & r)
		{
			Vector2 c(
				geom::DotProduct(v, gfx::GetAxis(r, gfx::Direction::right)),
				geom::DotProduct(v, gfx::GetAxis(r, gfx::Direction::up)));
			return std::atan2(c.x, c.y);
		};
		
		// an angle gleaned from difference between world positions
		const auto world_rotation_angle = get_angle(world_position_diff_direction, camera_rotation);
		
		// an angle gleaned from difference between screen positions
		const auto camera_to_world_ratio = camera_to_world_distance1 / camera_to_world_distance2;
		const auto normalized_screen_position2 = camera_to_screen_direction2 / camera_to_world_ratio;
		const auto normalized_screen_diff_direction = geom::Normalized(normalized_screen_position2 - camera_to_screen_direction1);
		const auto screen_rotation_angle = get_angle(normalized_screen_diff_direction, camera_rotation);
		
		// the difference between the world and screen rotations dictates output rotaiton
		const auto rotation_angle = world_rotation_angle - screen_rotation_angle;
		const auto output_rotation = camera_rotation * geom::Inverse(gfx::Rotation(gfx::Direction::forward, rotation_angle));
		
		// displacement from world position2 to the new camera position
		auto ws_camera_to_screen_direction2 = ((camera_to_screen_direction2 * camera_rotation) * gfx::Rotation(gfx::Direction::forward, rotation_angle) * geom::Inverse(camera_rotation));
		
		// put it all together and book in to local clinic
		const auto output_translation = geom::Cast<Scalar>(finger2.world_position) - ws_camera_to_screen_direction2 * camera_to_world_distance2;

		return sim::Transformation(geom::Cast<sim::Scalar>(output_translation), geom::Cast<sim::Scalar>(output_rotation));
	}
}

////////////////////////////////////////////////////////////////////////////////
// TouchObserverController member definitions

struct TouchObserverController::Finger
{
	// initial direction from camera to point of contact between finger and screen
	Vector3 world_position;
	
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
			auto screen_position = GetScreenPosition(event.tfinger);
			HandleFingerDown(screen_position, event.tfinger.fingerId);
			break;
		}
			
		case SDL_FINGERUP:
		{
			HandleFingerUp(event.tfinger.fingerId);
			break;
		}
			
		case SDL_FINGERMOTION:
		{
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
			auto& found_finger = _fingers.back();
			found_finger.id = id;
			return found_finger;
		}
		else
		{
			return * found;
		}
	} ();
		
	_down_transformation = GetTransformation();
	
	// generate ray from eye to finger
	auto direction = GetPixelDirection(screen_position, _down_transformation);
	Ray3 ray(_down_transformation.GetTranslation(), geom::Normalized(direction));

	// perform ray cast to determine what finger is pointing to
	auto touch_result = CastRay(ray, max_touch_ray_cast_distance);
	auto touch_distance = std::min(touch_result.GetDistance(), max_touch_ray_cast_distance);

	// store as the world position
	finger.world_position = geom::Project(ray, touch_distance);
	finger.screen_position = screen_position;
	
#if defined(SPAWN_SHAPES)
	auto speed = 0.f;
	auto velocity = direction * speed;
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

	_down_transformation = GetTransformation();

	ASSERT(! IsDown(id));
}

void TouchObserverController::HandleFingerMotion(Vector2 const & screen_position, SDL_FingerID id)
{
	auto found = FindFinger(id);
	if (found == _fingers.end())
	{
		// happens when a move event immediately follows an up event
		return;
	}
	
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
	Vector3 camera_position = _down_transformation.GetTranslation();
	Matrix33 camera_rotation = _down_transformation.GetRotation();
	Vector3 camera_up = gfx::GetAxis(camera_rotation, gfx::Direction::up);
	
	// get 'from' and 'to' finger pointing directions
	Vector3 relative_down_direction = geom::Normalized(finger.world_position - camera_position);
	Vector3 relative_current_direction = geom::Normalized(GetPixelDirection(finger.screen_position, _down_transformation));
	
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
	if (finger1.screen_position == finger2.screen_position
	|| finger1.world_position == finger2.world_position)
	{
		DEBUG_MESSAGE("finger collision detected!");
		return;
	}
	
	auto convert = [=] (Finger const & finger) 
	{
		TranslationRollFinger translation_roll_finger = 
		{
			finger.world_position,
			GetPixelDirection(finger.screen_position, _down_transformation)
		};
		
		return translation_roll_finger;
	};
	
	auto translation_roll_finger1 = convert(finger1);
	auto translation_roll_finger2 = convert(finger2);
	
	// the math
	auto camera_transformation = CalculateCameraTranslationRoll(translation_roll_finger1, translation_roll_finger2, _down_transformation);

	ClampTransformation(camera_transformation);

	SetTransformation(camera_transformation);
}

Transformation const & TouchObserverController::GetTransformation() const
{
	auto & entity = GetEntity();
	auto location = entity.GetLocation();
	ASSERT(location);
	
	return location->GetTransformation();
}

// adjusts given transformation to avoid penetrating world geometry
void TouchObserverController::ClampTransformation(Transformation & transformation) const
{
	// get start point
	auto destination_translation = transformation.GetTranslation();

	// create ray to proposed destination
	Ray3 ray;
	ray.position = _down_transformation.GetTranslation();
	ray.direction = destination_translation - ray.position;

	// normalize the ray
	auto ray_length = geom::Length(ray.direction);
	if (ray_length < .00001f)
	{
		// no change
		return;
	}
	ray.direction /= ray_length;

	// add a little extra to account for near z (unreliable)
	ray_length += 1.f;

	// how far alone the ray is clear?
	auto result = CastRay(ray, ray_length);
	if (! result)
	{
		// full length is fine
		return;
	}

	auto projection = std::max(1.f, result.GetDistance() - 1.f);
	transformation.SetTranslation(geom::Project(ray, projection));
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

form::RayCastResult TouchObserverController::CastRay(Ray3 const & ray, Scalar & length) const
{
	auto & entity = GetEntity();
	auto & engine = entity.GetEngine();
	auto & physics_engine = engine.GetPhysicsEngine();
	const auto body = entity.GetBody();
	
	return physics_engine.CastRay(ray, length, body);
}

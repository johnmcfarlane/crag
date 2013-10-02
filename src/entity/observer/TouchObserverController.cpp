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
#include "Contact.h"

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

CONFIG_DECLARE_ANGLE(camera_fov, float);
CONFIG_DECLARE(camera_near, float);

using namespace sim;

namespace
{
	CONFIG_DEFINE (max_touch_ray_cast_distance, physics::Scalar, 1000000000.f);
	CONFIG_DEFINE (touch_observer_min_distance, Scalar, 2.f);	// as a proportion of camera_near

	struct TranslationRollContact
	{
		Vector3 world_position;
		
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
		
			// hopefully a precision error
		if (sin_gamma > 1)
		{
			sin_gamma = 1.f;
		}
		else if (sin_gamma < -1)
		{
			sin_gamma = -1.f;
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
	
	Transformation CalculateCameraTranslationRoll(TranslationRollContact const & contact1, TranslationRollContact const & contact2, Transformation const & camera_transformation)
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
		P = contact1.screen_position
		Q = contact2.screen_position
		A = contact1.world_position
		B = contact2.world_position
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
		const auto screen_position_diff = geom::Cast<Scalar>(contact2.screen_position) - geom::Cast<Scalar>(contact1.screen_position);
		
		// distance between screen positions, PQ
		const auto screen_span = geom::Length(screen_position_diff);
		
		if (screen_span < .00001f)
		{
			return camera_transformation;
		}
		const auto screen_position_direction = screen_position_diff / screen_span;
		
		const auto camera_to_screen_distance1 = geom::Length(geom::Cast<Scalar>(contact1.screen_position));
		const auto camera_to_screen_distance2 = geom::Length(geom::Cast<Scalar>(contact2.screen_position));
		
		const auto camera_to_screen_direction1 = geom::Cast<Scalar>(contact1.screen_position) * Scalar(1) / camera_to_screen_distance1;
		const auto camera_to_screen_direction2 = geom::Cast<Scalar>(contact2.screen_position) * Scalar(1) / camera_to_screen_distance2;
		
		////////////////////////////////////////////////////////////////////////////////
		// world

		// vector from world_position #1 to world_position #2, AB
		const auto world_position_diff = geom::Cast<Scalar>(contact2.world_position) - geom::Cast<Scalar>(contact1.world_position);

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

		const auto camera_to_world_distance2 = world_position_forward2 * camera_to_screen_distance2;	// correct

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
		const auto camera_to_world_ratio = std::fabs(camera_to_world_distance1 / camera_to_world_distance2);
		const auto normalized_screen_position2 = camera_to_screen_direction2 / camera_to_world_ratio;
		const auto normalized_screen_diff_direction = geom::Normalized(normalized_screen_position2 - camera_to_screen_direction1);
		const auto screen_rotation_angle = get_angle(normalized_screen_diff_direction, camera_rotation);
		
		// the difference between the world and screen rotations dictates output rotaiton
		const auto rotation_angle = world_rotation_angle - screen_rotation_angle;
		const auto output_rotation = camera_rotation * geom::Inverse(gfx::Rotation(gfx::Direction::forward, rotation_angle));
		
		// displacement from world position2 to the new camera position
		auto ws_camera_to_screen_direction2 = ((camera_to_screen_direction2 * camera_rotation) * gfx::Rotation(gfx::Direction::forward, rotation_angle) * geom::Inverse(camera_rotation));
		
		// put it all together and book in to local clinic
		const auto output_translation = geom::Cast<Scalar>(contact2.world_position) - ws_camera_to_screen_direction2 * camera_to_world_distance2;

		return sim::Transformation(geom::Cast<sim::Scalar>(output_translation), geom::Cast<sim::Scalar>(output_rotation));
	}
}

////////////////////////////////////////////////////////////////////////////////
// TouchObserverController member definitions

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

	auto previous_transformation = GetTransformation();
	auto transformation = UpdateCamera(previous_transformation);
	
	ClampTransformation(transformation);

	if (previous_transformation != transformation)
	{
		SetTransformation(transformation);
	}

	BroadcastTransformation();
}

void TouchObserverController::operator() (gfx::SetOriginEvent const & event)
{
	// convert entity position
	auto old_transformation = GetTransformation();
	auto new_transformation = geom::Convert(old_transformation, _origin, event.origin);
	SetTransformation(new_transformation);
	
	// convert _down_transformation
	if (! _contacts.empty())
	{
		_down_transformation = geom::Convert(_down_transformation, _origin, event.origin);
	}
	
	// convert contacts
	for (auto & contact : _contacts)
	{
		contact = ConvertOrigin(contact, _origin, event.origin);
	}

	_origin = event.origin;
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
	// find existing / create new contact in _contacts and return reference
	auto& contact = [&] () -> Contact&
	{
		auto found = FindContact(id);

		if (found == _contacts.end())
		{
			_contacts.emplace_back();
			auto& found_contact = _contacts.back();
			return found_contact;
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
	
	// if ray hit something,
	if (touch_result)
	{
		auto touch_distance = touch_result.GetDistance();

		// store as the world position
		contact = Contact(geom::Project(ray, touch_distance), Contact::VectorType::foreground, screen_position, id);
	}
	else
	{
		// either way, store screen position and finger ID
		contact = Contact(ray.direction, Contact::VectorType::background, screen_position, id);
	}
	
#if defined(SPAWN_SHAPES)
	auto speed = 0.f;
	auto velocity = direction * speed;
	switch (_contacts.size())
	{
		case 1:
			SpawnBall(Sphere3(contact.GetWorldPosition(), 0.5f), velocity, gfx::Color4f::Red());
			break;
			
		case 2:
			SpawnBox(contact.GetWorldPosition(), velocity, Vector3(1, 1, 1), gfx::Color4f::Blue());
			break;
	}
#endif

	ASSERT(! _contacts.empty());
	ASSERT(IsDown(id));
}

void TouchObserverController::HandleFingerUp(SDL_FingerID id)
{
	ASSERT(! _contacts.empty());
	
	auto found = FindContact(id);
	if (found == _contacts.end())
	{
		DEBUG_MESSAGE("double-up detected");
	}
	else
	{
		_contacts.erase(found);
	}

	_down_transformation = GetTransformation();

	ASSERT(! IsDown(id));
}

void TouchObserverController::HandleFingerMotion(Vector2 const & screen_position, SDL_FingerID id)
{
	auto found = FindContact(id);
	if (found == _contacts.end())
	{
		// happens when a move event immediately follows an up event
		return;
	}
	
	found->SetScreenPosition(screen_position);
}

Transformation TouchObserverController::UpdateCamera(Transformation const & previous_transformation) const
{
	switch (_contacts.size())
	{
		case 0:
			return previous_transformation;
			
		case 1:
			return UpdateCamera(previous_transformation, _contacts[0]);
			
		default:
		{
			auto size = _contacts.size();
			const auto & contact0 = _contacts[size - 2];
			const auto & contact1 = _contacts[size - 1];
			return UpdateCamera(previous_transformation, {{ & contact0, & contact1 }});
		}
	}
}

// updates camera when one finger is down; 
// rotates the camera around a fixed position with fixed up axis 
// when one finger is touching the screen; 
// is intended to feel analogous to scrolling within a UI view
Transformation TouchObserverController::UpdateCamera(Transformation const &, Contact const & contact) const
{
	// extract data about initial camera transformation
	Vector3 camera_position = _down_transformation.GetTranslation();
	Matrix33 camera_rotation = _down_transformation.GetRotation();
	Vector3 camera_up = gfx::GetAxis(camera_rotation, gfx::Direction::up);
	
	// get 'from' and 'to' finger pointing directions
	Vector3 relative_down_direction = contact.GetWorldDirection(camera_position);
	Vector3 relative_current_direction = geom::Normalized(GetPixelDirection(contact.GetScreenPosition(), _down_transformation));
	
	// convert them into rotations and take the difference
	auto from_matrix = gfx::Rotation(relative_down_direction, camera_up);
	auto to_matrix = gfx::Rotation(relative_current_direction, camera_up);
	auto required_rotation = Inverse(from_matrix) * (to_matrix);
	
	// apply them to camera rotation
	// TODO: Is operator* the right way around?
	camera_rotation = required_rotation * camera_rotation;

	// apply new camera rotation
	Transformation camera_transformation(camera_position, Matrix33(camera_rotation));
	return camera_transformation;
}

// updates camera when two fingers are down; 
// zooms/pans/rolls the camera when one finer is touching the screen;
// is intended to feel analogous to zooming and rotating a UI view:
// - moving fingers in parallel pans camera
// - rotating fingers rotated the camera rolls the camera / rotates along the z
// - moving fingers together/appart translates the camera along the z
Transformation TouchObserverController::UpdateCamera(Transformation const & previous_transformation, std::array<Contact const *, 2> contacts) const
{
	ASSERT(contacts[0]);
	ASSERT(contacts[1]);

	if (Collided(* contacts[0], * contacts[1]))
	{
		DEBUG_MESSAGE("finger collision detected!");
		return previous_transformation;
	}
	
	struct ContactBuffer
	{
		std::array<Contact const *, 2> contacts = {{ nullptr, nullptr }};
		std::size_t size = 0;
	};
	
	std::array<ContactBuffer, 2> buffers;
	
	for (auto i = 0; i < 2; ++ i)
	{
		auto const & contact = * contacts[i];
		auto vector_type = contact.GetVectorType();
		auto buffer_index = static_cast<std::size_t>(vector_type);
		auto & buffer = buffers[buffer_index];
		buffer.contacts[buffer.size ++] = & contact;
	}
	
	auto ConvertForeground = [&] (Contact const & contact) 
	{
		TranslationRollContact translation_roll_contact = 
		{
			contact.GetWorldPosition(),
			GetPixelDirection(contact.GetScreenPosition(), _down_transformation)
		};

		return translation_roll_contact;
	};
	
	auto ConvertBackground = [&] (Contact const & background_contact, Vector3 const & foreground_contact_position)
	{
		auto camera_position = _down_transformation.GetTranslation();
		auto distance = geom::Length(foreground_contact_position - camera_position);

		TranslationRollContact translation_roll_contact = 
		{
			geom::Project(Ray3(camera_position, background_contact.GetWorldDirection()), distance),
			GetPixelDirection(background_contact.GetScreenPosition(), _down_transformation)
		};

		return translation_roll_contact;
	};
	
	auto const & foreground_buffer = buffers[static_cast<std::size_t>(Contact::VectorType::foreground)];
	auto const & background_buffer = buffers[static_cast<std::size_t>(Contact::VectorType::background)];

	std::array<TranslationRollContact, 2> translation_roll_contacts;
	
	switch (foreground_buffer.size)
	{
		case 2:
		{
			translation_roll_contacts[0] = ConvertForeground(* foreground_buffer.contacts[0]);
			translation_roll_contacts[1] = ConvertForeground(* foreground_buffer.contacts[1]);
			break;
		}

		case 1:
		{
			translation_roll_contacts[0] = ConvertForeground(* foreground_buffer.contacts[0]);
			translation_roll_contacts[1] = ConvertBackground(* background_buffer.contacts[0], translation_roll_contacts[0].world_position);
			break;
		}
		
		case 0:
			return previous_transformation;
	}

	auto camera_transformation = CalculateCameraTranslationRoll(translation_roll_contacts[0], translation_roll_contacts[1], _down_transformation);
	return camera_transformation;
}

Transformation const & TouchObserverController::GetTransformation() const
{
	auto & entity = GetEntity();
	auto location = entity.GetLocation();
	ASSERT(location);
	
	return location->GetTransformation();
}

// returns true if transformation was mutated
bool TouchObserverController::ClampTransformation(Transformation & transformation) const
{
	int i = 0;
	while (TryClampTransformation(transformation) && ++ i < 10);
	return i > 0;
}

// adjusts given transformation to avoid penetrating world geometry;
// returns true if world geometry /might/ be encroaching on near render plane
bool TouchObserverController::TryClampTransformation(Transformation & transformation) const
{
	Vector3 camera_position = transformation.GetTranslation();
	Scalar max_clear_distance = camera_near * touch_observer_min_distance;
	Scalar min_clear_distance = camera_near;
	Sphere3 collision_sphere(camera_position, max_clear_distance);

	auto & entity = GetEntity();
	auto & engine = entity.GetEngine();
	auto & physics_engine = engine.GetPhysicsEngine();
	
	auto push = Vector3::Zero();
	Scalar max_push_distance = 0;
	auto function = [& push, & max_push_distance] (physics::ContactGeom const * begin, physics::ContactGeom const * end)
	{
		ASSERT(end > begin);
		do
		{
			auto push_distance = begin->depth;
			if (push_distance > max_push_distance)
			{
				max_push_distance = push_distance;
				push = physics::Convert(begin->normal);
			}
		}
		while (++ begin != end);
	};

	physics::ContactFunction<decltype(function)> contact_function(function);
	physics_engine.Collide(collision_sphere, contact_function);
	
	if (max_push_distance > 0)
	{
		transformation.SetTranslation(camera_position + push * max_push_distance);
		return max_push_distance > max_clear_distance - min_clear_distance;
	}
	
	return false;
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

TouchObserverController::ContactVector::iterator TouchObserverController::FindContact(SDL_FingerID id)
{
	return std::find_if(_contacts.begin(), _contacts.end(), [id] (Contact const & contact) 
	{
		return contact.GetId() == id;
	});
}

TouchObserverController::ContactVector::const_iterator TouchObserverController::FindContact(SDL_FingerID id) const
{
	return const_cast<TouchObserverController&>(*this).FindContact(id);
}

bool TouchObserverController::IsDown(SDL_FingerID id) const
{
	return FindContact(id) != _contacts.end();
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

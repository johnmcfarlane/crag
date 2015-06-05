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
#include "gfx/Pov.h"
#include "gfx/SetCameraEvent.h"
#include "gfx/SetLodParametersEvent.h"
#include "gfx/SetSpaceEvent.h"

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/Random.h"
#include "core/RosterObjectDefine.h"

#include "geom/Quaternion.h"

#if defined(CRAG_DEBUG)
//#define SPAWN_SHAPES
#endif

#if defined(SPAWN_SHAPES)
#include "scripts/SpawnEntityFunctions.h"
#include "gfx/Color.h"
#endif

using namespace sim;

namespace
{
	CONFIG_DEFINE(max_touch_ray_cast_distance, 1000000000.f);
	CONFIG_DEFINE(touch_observer_distance_buffer, 2.f);	// as a proportion of near Z

	struct TranslationRollContact
	{
		Vector3 world_position;
		
		// vector from camera to touch-point on screen, so no shorter than unit length
		Vector3 screen_position;
		
		static TranslationRollContact Zero();
	};

	TranslationRollContact TranslationRollContact::Zero()
	{
		return TranslationRollContact {
			Vector3::Zero(),
			Vector3::Zero()
		};
	}

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
			CRAG_VERIFY_OP(std::fabs(b), >=, std::fabs(c));
			CRAG_VERIFY_OP(gamma, <=, S(PI * .5));
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
		const auto screen_span = geom::Magnitude(screen_position_diff);
		
		if (screen_span < .00001f)
		{
			return camera_transformation;
		}
		const auto screen_position_direction = screen_position_diff / screen_span;
		
		const auto camera_to_screen_distance1 = geom::Magnitude(geom::Cast<Scalar>(contact1.screen_position));
		const auto camera_to_screen_distance2 = geom::Magnitude(geom::Cast<Scalar>(contact2.screen_position));
		
		const auto camera_to_screen_direction1 = geom::Cast<Scalar>(contact1.screen_position) * Scalar(1) / camera_to_screen_distance1;
		const auto camera_to_screen_direction2 = geom::Cast<Scalar>(contact2.screen_position) * Scalar(1) / camera_to_screen_distance2;
		
		////////////////////////////////////////////////////////////////////////////////
		// world

		// vector from world_position #1 to world_position #2, AB
		const auto world_position_diff = geom::Cast<Scalar>(contact2.world_position) - geom::Cast<Scalar>(contact1.world_position);

		// how far forwards of world position #1 is #2, d
		const auto world_position_diff_forward = geom::DotProduct(world_position_diff, camera_forward);

		// distance between world positions, |AB| aka c
		const auto world_span = geom::Magnitude(world_position_diff);
		
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

CRAG_ROSTER_OBJECT_DEFINE(
	TouchObserverController,
	1,
	Pool::CallBefore<& TouchObserverController::Tick, Entity, & Entity::Tick>(Engine::GetTickRoster()))

TouchObserverController::TouchObserverController(Entity & entity, Transformation const & transformation)
: Controller(entity)
, _down_transformation(transformation)
, _current_transformation(transformation)
{
	_frustum.resolution = app::GetResolution();
	_frustum.depth_range = Vector2(1, 2);
}

TouchObserverController::~TouchObserverController()
{
	SetIsListening(false);
}

void TouchObserverController::Tick()
{
	// event-based input
	HandleEvents();

	UpdateCamera();
	ClampTransformation();
	BroadcastTransformation();
}

void TouchObserverController::operator() (gfx::SetSpaceEvent const & event)
{
	// convert _down_transformation
	_down_transformation = geom::Convert(_down_transformation, _space, event.space);
	
	// convert _current_transformation
	_current_transformation = geom::Convert(_current_transformation, _space, event.space);
	
	// convert contacts
	for (auto & contact : _contacts)
	{
		contact = ConvertSpace(contact, _space, event.space);
	}

	_space = event.space;
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
	auto & contact = [&] () -> Contact&
	{
		auto found = FindContact(id);

		if (found == _contacts.end())
		{
			_contacts.emplace_back();
			auto & found_contact = _contacts.back();
			return found_contact;
		}
		else
		{
			return * found;
		}
	} ();
		
	_down_transformation = _current_transformation;
	
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
	auto found = FindContact(id);
	if (found == _contacts.end())
	{
		// the very first event is often an 'up' because an icon was just tapped!
		return;
	}

	_contacts.erase(found);

	_down_transformation = _current_transformation;

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

void TouchObserverController::UpdateCamera()
{
	switch (_contacts.size())
	{
		case 0:
			break;
			
		case 1:
			UpdateCamera(_contacts[0]);
			break;
			
		default:
		{
			auto size = _contacts.size();
			const auto & contact0 = _contacts[size - 2];
			const auto & contact1 = _contacts[size - 1];
			UpdateCamera({{ & contact0, & contact1 }});
			break;
		}
	}
}

// updates camera when one finger is down; 
// rotates the camera around a fixed position with fixed up axis 
// when one finger is touching the screen; 
// is intended to feel analogous to scrolling within a UI view
void TouchObserverController::UpdateCamera(Contact const & contact)
{
	// extract data about initial camera transformation
	Vector3 const down_position = _down_transformation.GetTranslation();
	Matrix33 const down_rotation = _down_transformation.GetRotation();
	Vector3 const camera_up = gfx::GetAxis(down_rotation, gfx::Direction::up);
	
	// get 'from' and 'to' finger pointing directions
	Vector3 relative_down_direction = contact.GetWorldDirection(down_position);
	Vector3 relative_current_direction = geom::Normalized(GetPixelDirection(contact.GetScreenPosition(), _down_transformation));
	
	// convert them into rotations and take the difference
	auto from_matrix = gfx::Rotation(relative_down_direction, camera_up);
	auto to_matrix = gfx::Rotation(relative_current_direction, camera_up);
	auto required_rotation = Inverse(from_matrix) * (to_matrix);
	
	// apply them to camera rotation
	Matrix33 new_rotation = required_rotation * down_rotation;
	if (new_rotation != _current_transformation.GetRotation())
	{
		// apply new camera rotation
		_current_transformation.SetRotation(new_rotation);
	}
}

// updates camera when two fingers are down; 
// zooms/pans/rolls the camera when one finer is touching the screen;
// is intended to feel analogous to zooming and rotating a UI view:
// - moving fingers in parallel pans camera
// - rotating fingers rotated the camera rolls the camera / rotates along the z
// - moving fingers together/appart translates the camera along the z
void TouchObserverController::UpdateCamera(std::array<Contact const *, 2> contacts)
{
	ASSERT(contacts[0]);
	ASSERT(contacts[1]);

	if (Collided(* contacts[0], * contacts[1]))
	{
		DEBUG_MESSAGE("finger collision detected!");
		return;
	}
	
	struct ContactBuffer
	{
		ContactBuffer()
			: contacts({ { nullptr, nullptr } })
			, size(0)
		{
		}

		std::array<Contact const *, 2> contacts;
		std::size_t size;
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
			this->GetPixelDirection(contact.GetScreenPosition(), this->_down_transformation)
		};

		return translation_roll_contact;
	};
	
	auto ConvertBackground = [&] (Contact const & background_contact, Vector3 const & foreground_contact_position)
	{
		auto camera_position = this->_down_transformation.GetTranslation();
		auto distance = geom::Magnitude(foreground_contact_position - camera_position);

		TranslationRollContact translation_roll_contact = 
		{
			geom::Project(Ray3(camera_position, background_contact.GetWorldDirection()), distance),
			this->GetPixelDirection(background_contact.GetScreenPosition(), this->_down_transformation)
		};

		return translation_roll_contact;
	};
	
	auto const & foreground_buffer = buffers[static_cast<std::size_t>(Contact::VectorType::foreground)];
	auto const & background_buffer = buffers[static_cast<std::size_t>(Contact::VectorType::background)];

	std::array<TranslationRollContact, 2> translation_roll_contacts =
	{{ 
		TranslationRollContact::Zero(),
		TranslationRollContact::Zero()
	}};
	
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
			return;
	}

	auto camera_transformation = CalculateCameraTranslationRoll(translation_roll_contacts[0], translation_roll_contacts[1], this->_down_transformation);
	if (camera_transformation != this->_current_transformation)
	{
		this->_current_transformation = camera_transformation;
	}
}

// adjusts given transformation to avoid penetrating world geometry;
void TouchObserverController::ClampTransformation()
{
	auto push = physics::Vector3::Zero();
	physics::Scalar max_push_distance = 0;

	auto contact_function = physics::ContactFunction([&] (physics::ContactGeom const * begin, physics::ContactGeom const * end) {
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
		while ((++ begin) != end);
	});

	auto camera_near = _frustum.depth_range[0];

	for (auto pass = 5; pass; -- pass)
	{
		ASSERT(touch_observer_distance_buffer >= 1.f);
	
		Vector3 center = _current_transformation.GetTranslation();
		Scalar radius = camera_near * touch_observer_distance_buffer;
		Sphere3 collision_sphere(center, radius);

		auto & entity = GetEntity();
		auto & engine = entity.GetEngine();
		auto & physics_engine = engine.GetPhysicsEngine();
	
		physics_engine.Collide(geom::Cast<physics::Scalar>(collision_sphere), contact_function);
	
		if (max_push_distance <= 0)
		{
			break;
		}
		
		_current_transformation.SetTranslation(center + geom::Cast<Scalar>(push * max_push_distance));
	}
}

void TouchObserverController::BroadcastTransformation() const
{
	// broadcast new camera position
	gfx::SetCameraEvent set_camera_event = {
		_space.RelToAbs(_current_transformation),
		_frustum.fov
	};
	Daemon::Broadcast(set_camera_event);

	// broadcast new lod center
	gfx::SetLodParametersEvent set_lod_parameters_event;
	set_lod_parameters_event.parameters.center = _current_transformation.GetTranslation();
	set_lod_parameters_event.parameters.min_distance = _frustum.depth_range[0];
	Daemon::Broadcast(set_lod_parameters_event);
}

physics::Body & TouchObserverController::GetBody()
{
	auto & entity = GetEntity();
	auto & location = * entity.GetLocation();
	return core::StaticCast<physics::Body>(location);
}

physics::Body const & TouchObserverController::GetBody() const
{
	auto const & entity = GetEntity();
	auto const & location = * entity.GetLocation();
	return core::StaticCast<physics::Body const>(location);
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
	gfx::Pov pov(transformation, _frustum);

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
	auto const & location = * entity.GetLocation();
	auto const & body = core::StaticCast<physics::Body const>(location);
	
	return physics_engine.CastRay(ray, length, & body);
}

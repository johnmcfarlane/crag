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

#include "core/app.h"
#include "core/ConfigEntry.h"
#include "core/Roster.h"

using namespace sim;

////////////////////////////////////////////////////////////////////////////////
// TouchObserverController member definitions

TouchObserverController::TouchObserverController(Entity & entity, Vector3 const & position)
: Controller(entity)
, _position(position)
{
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.AddOrdering(& TouchObserverController::Tick, & Entity::Tick);
	roster.AddCommand(* this, & TouchObserverController::Tick);
}

TouchObserverController::~TouchObserverController()
{
	// roster
	auto & roster = GetEntity().GetEngine().GetTickRoster();
	roster.RemoveCommand(* this, & TouchObserverController::Tick);
}

void TouchObserverController::Tick()
{
	// event-based input
	HandleEvents();
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
		case SDL_FINGERDOWN:
			HandleFingerDown(Vector2(event.tfinger.x, event.tfinger.y), event.tfinger.fingerId);
			break;
			
		case SDL_FINGERUP:
			HandleFingerUp(event.tfinger.fingerId);
			break;
			
		case SDL_FINGERMOTION:
			HandleFingerMotion(Vector2(event.tfinger.x, event.tfinger.y), event.tfinger.fingerId);
			break;
		
		default:
			break;
	}
}

void TouchObserverController::HandleFingerDown(Vector2 const & normalized_position, SDL_FingerID id)
{
	auto position = NormalizedToScreen(normalized_position);
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
	
	finger.down_position = position;
	finger.position = position;
	
	ASSERT(FindFinger(id) != _fingers.end());
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
	
	ASSERT(FindFinger(id) == _fingers.end());
}

void TouchObserverController::HandleFingerMotion(Vector2 const & normalized_position, SDL_FingerID id)
{
	auto found = FindFinger(id);
	if (found == _fingers.end())
	{
		DEBUG_MESSAGE("missing down");
		HandleFingerDown(normalized_position, id);
		return;
	}
	
	auto position = NormalizedToScreen(normalized_position);
	found->position = position;
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

Vector2 TouchObserverController::NormalizedToScreen(Vector2 normalized_position)
{
	auto resolution = geom::Cast<float>(app::GetResolution());
	return resolution * normalized_position;
}

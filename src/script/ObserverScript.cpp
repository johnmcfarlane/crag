//
//  ObserverScript.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-11.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ObserverScript.h"

#include "ScriptThread.h"

#include "sim/EntityFunctions.h"
#include "sim/Observer.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::ObserverScript member definitions

ObserverScript::ObserverScript(sim::Vector3 const & spawn_position)
: _collidable(true)
{
	_observer.Create(spawn_position);
};

ObserverScript::~ObserverScript()
{
	_observer.Destroy();
}

void ObserverScript::operator() (FiberInterface & fiber)
{
	DEBUG_MESSAGE("-> ObserverScript");
	
	while (! fiber.GetQuitFlag())
	{
		fiber.Wait(_event_condition);
		SDL_Event const & event = _event_condition.GetEvent();
		
		if (! HandleEvent(event))
		{
			fiber.SetQuitFlag();
		}
	}

	DEBUG_MESSAGE("<- ObserverScript");
}

// returns false if it's time to quit
bool ObserverScript::HandleEvent(SDL_Event const & event)
{
	switch (event.type)
	{
		case SDL_QUIT:
			return false;
			
		case SDL_KEYDOWN:
			return HandleKeyboardEvent(event.key.keysym.scancode, 1);
			
		case SDL_KEYUP:
			return HandleKeyboardEvent(event.key.keysym.scancode, 0);
			
		case SDL_MOUSEBUTTONDOWN:
			return HandleMouseButton(event.button.button, 1);
			
		case SDL_MOUSEBUTTONUP:
			return HandleMouseButton(event.button.button, 0);
			
		case SDL_MOUSEMOTION:
			return HandleMouseMove(event.motion.xrel, event.motion.yrel);
			
		default:
			return true;
	}
}

namespace
{
	class SetCollidableFunctor
	{
	public:
		SetCollidableFunctor(bool collidable)
		: _collidable(collidable)
		{
		}
		
		void operator()(sim::Entity * entity) const
		{
			if (entity == nullptr)
			{
				ASSERT(false);
				return;
			}
			
			sim::SetCollidable(* entity, _collidable);
		}
		
	private:
		bool _collidable;
	};
}

// returns false if it's time to quit
bool ObserverScript::HandleKeyboardEvent(SDL_Scancode scancode, bool down)
{
	if (down)
	{
		switch (scancode)
		{
			case SDL_SCANCODE_ESCAPE:
				return false;
				
			case SDL_SCANCODE_C:
				{
					_collidable = ! _collidable;
					SetCollidableFunctor functor(_collidable);
					_observer.Call(functor);
				}
				return true;

			case SDL_SCANCODE_0:
				SetSpeed(10);
				return true;
				
			default:
				if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
				{
					SetSpeed(scancode + 1 - SDL_SCANCODE_1);
				}
				return true;
		}
	}
	
	return true;
}

bool ObserverScript::HandleMouseButton(Uint8 button, bool down)
{
	return true;
}

namespace
{
	class AddRotationFunctor
	{
	public:
		AddRotationFunctor(Vector3f const & rotation)
		: _rotation(rotation)
		{
		}
		
		void operator()(sim::Observer * observer) const
		{
			if (observer == nullptr)
			{
				ASSERT(false);
				return;
			}
			
			observer->AddRotation(_rotation);
		}
		
	private:
		Vector3f _rotation;
	};
}

bool ObserverScript::HandleMouseMove(int x_delta, int y_delta)
{
	float sensitivity = 0.1f;
	
	Vector3f rotation;
	rotation.x = - y_delta * sensitivity;
	rotation.y = 0;
	rotation.z = - x_delta * sensitivity;

	AddRotationFunctor functor(rotation);
	_observer.Call(functor);	
	
	return true;
}

class SetSpeedFunctor
{
public:
	SetSpeedFunctor(int __speed)
	: _speed(__speed)
	{
	}
	
	void operator()(sim::Observer * observer) const
	{
		if (observer == nullptr)
		{
			ASSERT(false);
			return;
		}
		
		observer->SetSpeed(_speed);
	}
	
private:
	int _speed;
};

void ObserverScript::SetSpeed(int speed)
{
	SetSpeedFunctor functor(speed);
	_observer.Call(functor);
}

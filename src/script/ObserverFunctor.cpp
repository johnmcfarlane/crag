//
//  ObserverFunctor.cpp
//  crag
//
//  Created by John McFarlane on 2012-02-11.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "ObserverFunctor.h"

#include "ScriptThread.h"

#include "sim/EntityFunctions.h"


using namespace script;


////////////////////////////////////////////////////////////////////////////////
// script::ObserverFunctor member definitions

ObserverFunctor::ObserverFunctor(sim::Vector3 const & spawn_position)
: _collidable(true)
{
	sim::InitData<sim::Observer> init_data;
	init_data.center = spawn_position;
	_observer.Create(init_data);
};

ObserverFunctor::~ObserverFunctor()
{
	_observer.Destroy();
}

void ObserverFunctor::operator() (FiberInterface & fiber)
{
	std::cout << "-> ObserverFunctor" << std::endl;
	
	while (! fiber.GetQuitFlag())
	{
		fiber.Wait(_event_condition);
		SDL_Event const & event = _event_condition.GetEvent();
		
		if (! HandleEvent(event))
		{
			fiber.SetQuitFlag();
		}
	}

	std::cout << "<- ObserverFunctor" << std::endl;
}

// returns false if it's time to quit
bool ObserverFunctor::HandleEvent(SDL_Event const & event)
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
				Assert(false);
				return;
			}
			
			sim::SetCollidable(* entity, _collidable);
		}
		
	private:
		bool _collidable;
	};
}

// returns false if it's time to quit
bool ObserverFunctor::HandleKeyboardEvent(SDL_Scancode scancode, bool down)
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

bool ObserverFunctor::HandleMouseButton(Uint8 button, bool down)
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
				Assert(false);
				return;
			}
			
			observer->AddRotation(_rotation);
		}
		
	private:
		Vector3f _rotation;
	};
}

bool ObserverFunctor::HandleMouseMove(int x_delta, int y_delta)
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
			Assert(false);
			return;
		}
		
		observer->SetSpeed(_speed);
	}
	
private:
	int _speed;
};

void ObserverFunctor::SetSpeed(int speed)
{
	SetSpeedFunctor functor(speed);
	_observer.Call(functor);
}

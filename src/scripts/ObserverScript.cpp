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

#include "sim/Engine.h"
#include "sim/EntityFunctions.h"
#include "sim/Observer.h"

#include "applet/AppletInterface_Impl.h"
#include "applet/Engine.h"

#include "core/ConfigEntry.h"

using namespace applet;

// TODO: This value is likely sensitive to screen resolutions
#if defined(__APPLE__)
CONFIG_DEFINE (mouse_sensitivity, float, 0.1f);
#elif defined(WIN32)
CONFIG_DEFINE (mouse_sensitivity, float, 0.05f);
#else
CONFIG_DEFINE (mouse_sensitivity, float, 0.01f);
#endif

////////////////////////////////////////////////////////////////////////////////
// applet::ObserverScript member definitions

ObserverScript::ObserverScript(AppletBase::Init const & init, sim::ObserverHandle observer)
: AppletBase(init, 4096, "Observer")
, _observer(observer)
, _collidable(true)
{
}

ObserverScript::~ObserverScript()
{
	_observer.Destroy();
}

void ObserverScript::operator() (AppletInterface & applet_interface)
{
	DEBUG_MESSAGE("-> ObserverScript");
	
	while (! applet_interface.GetQuitFlag())
	{
		HandleEvents(applet_interface);
	}

	DEBUG_MESSAGE("<- ObserverScript");
}

void ObserverScript::HandleEvents(AppletInterface & applet_interface)
{
	SDL_Event event;
	if (_event_watcher.PopEvent(event))
	{
		HandleEvent(event);
	}
	else
	{
		applet_interface.WaitFor([this, & applet_interface] () {
			return ! _event_watcher.IsEmpty() || applet_interface.GetQuitFlag();
		});
	}
}

void ObserverScript::HandleEvent(SDL_Event const & event)
{
	switch (event.type)
	{
		case SDL_KEYDOWN:
			HandleKeyboardEvent(event.key.keysym.scancode, 1);
			break;
			
		case SDL_KEYUP:
			HandleKeyboardEvent(event.key.keysym.scancode, 0);
			break;
			
		case SDL_MOUSEBUTTONDOWN:
			HandleMouseButton(event.button.button, 1);
			break;
			
		case SDL_MOUSEBUTTONUP:
			HandleMouseButton(event.button.button, 0);
			break;
			
		case SDL_MOUSEMOTION:
			HandleMouseMove(event.motion.xrel, event.motion.yrel);
			break;
			
		default:
			break;
	}
}

// returns false if it's time to quit
void ObserverScript::HandleKeyboardEvent(SDL_Scancode scancode, bool down)
{
	if (down)
	{
		switch (scancode)
		{
			case SDL_SCANCODE_C:
				{
					_collidable = ! _collidable;
					bool collidable = _collidable;
					_observer.Call([collidable] (sim::Entity & entity) {
						sim::SetCollidable(entity, collidable);
					});
				}
				break;

			case SDL_SCANCODE_0:
				SetSpeed(10);
				break;
				
			default:
				if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_9)
				{
					SetSpeed(scancode + 1 - SDL_SCANCODE_1);
				}
				break;
		}
	}
}

void ObserverScript::HandleMouseButton(Uint8 button, bool down)
{
}

void ObserverScript::HandleMouseMove(int x_delta, int y_delta)
{
	sim::Vector3 rotation;
	rotation.x = - y_delta * mouse_sensitivity;
	rotation.y = 0;
	rotation.z = - x_delta * mouse_sensitivity;

	_observer.Call([rotation] (sim::Observer & observer) {
		observer.AddRotation(rotation);
	});
}

void ObserverScript::SetSpeed(int speed)
{
	_observer.Call([speed] (sim::Observer & observer) {
		observer.SetSpeed(speed);
	});
}

//
//  ObserverScript.h
//  crag
//
//  Created by John McFarlane on 2012-02-11.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "AppletBase.h"

#include "sim/defs.h"


DECLARE_CLASS_HANDLE(sim, Observer)	// sim::ObserverHandle


namespace applet
{
	class ObserverScript : public AppletBase
	{
	public:
		// functions
		ObserverScript(sim::Vector3 const & spawn_position);
		~ObserverScript();
		
		virtual void operator() (AppletInterface & applet_interface) override;
		
	private:
		void HandleEvent(SDL_Event const & event);
		void HandleKeyboardEvent(SDL_Scancode scancode, bool down);
		void HandleMouseButton(Uint8 button, bool down);
		void HandleMouseMove(int x_delta, int y_delta);
		
		void SetSpeed(int speed);
		void InitiateShutdown();
		
		// variables
		sim::ObserverHandle _observer;
		bool _collidable;
	};
}

//
//  entity/sim/UfoController2.h
//  crag
//
//  Created by John McFarlane on 2014-07-23.
//	Copyright 2014 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/Controller.h"
#include "sim/Engine.h"

#include "gfx/Pov.h"
#include "gfx/SetCameraEvent.h"

#include "ipc/Listener.h"

namespace sim
{
	// controls a vessel with mouse or touch-based tilting behavior
	class UfoController2 final
	: public Controller
	, private ipc::Listener<Engine, gfx::SetCameraEvent>
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		UfoController2(Entity & entity, std::shared_ptr<Entity> const & ball_entity, Scalar thrust);
		~UfoController2();

		CRAG_VERIFY_INVARIANTS_DECLARE(UfoController2);

	private:
		void operator() (gfx::SetCameraEvent const & event);

		static void Tick(UfoController2 *);

		////////////////////////////////////////////////////////////////////////////////
		// data

		gfx::Pov _pov;
		
		std::shared_ptr<Entity> _ball_entity;
		
		float _thrust;
	};
}

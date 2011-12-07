//
//  sim/Ball.h
//  crag
//
//  Created by John McFarlane on 6/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Entity.h"

#include "gfx/defs.h"


namespace gfx
{
	class Ball;
}


namespace sim
{
	class Ball : public Entity
	{
		DECLARE_SCRIPT_CLASS(Ball, Entity);
		
	public:
		~Ball();
		
		// Type-specific allocation via script.
		static void Create(Ball & ball, PyObject & args);
		
		// Called from the simulation thread.
		bool Init(Simulation & simulation, PyObject & args);
	private:
		void InitPhysics(Simulation & simulation, Sphere3 const & sphere);
		void InitGraphics(Sphere3 const & sphere);
		
	public:
		void UpdateModels() const;
		
	protected:
		gfx::Uid GetGfxUid() const;

	private:
		// variables
		gfx::Uid _gfx_uid;
	};
}

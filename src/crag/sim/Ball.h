//
//  Ball.h
//  crag
//
//  Created by John McFarlane on 6/23/11.
//	Copyright 2009-2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Entity.h"


namespace physics
{
	class SphericalBody;
}

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
		Ball();
		~Ball();
		
		// Type-specific allocation via script.
		static void Create(Ball & ball, PyObject & args);
		
		// Called from the simulation thread.
		bool Init(Simulation & simulation, PyObject & args);
		
		void Tick(Simulation & simulation);
		void UpdateModels() const;
	private:
		virtual Vector3 const & GetPosition() const;
		
		// variables
		physics::SphericalBody * _body;
		gfx::Ball * _model;
	};
}
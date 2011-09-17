//
//  Box.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Entity.h"


namespace physics
{
	class BoxBody;
}

namespace gfx
{
	class Box;
}


namespace sim
{
	class Box : public Entity
	{
		DECLARE_SCRIPT_CLASS(Box, Entity);
		
	public:
		Box();
		~Box();
		
		// Type-specific allocation via script.
		static void Create(Box & box, PyObject & args);
		
		// Called from the simulation thread.
		bool Init(Simulation & simulation, PyObject & args);
		
		void Tick(Simulation & simulation);
		void UpdateModels() const;
	private:
		virtual Vector3 const & GetPosition() const;
		
		// variables
		physics::BoxBody * _body;
		gfx::Box * _model;
	};
}
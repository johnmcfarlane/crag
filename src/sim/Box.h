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

#include "gfx/defs.h"


namespace physics
{
	class BoxBody;
}


namespace sim
{
	// a cuboid-shaped entity
	class Box : public Entity
	{
	public:
		Box();
		~Box();
		
		// Called from the simulation thread.
		void Init(Simulation & simulation, InitData<Box> const & init_data);
	private:
		void InitPhysics(Simulation & simulation, Vector3 center, Vector3 size);
		void InitGraphics();
		
	public:
		void UpdateModels() const;
	private:
		
		// variables
		gfx::Uid _gfx_uid;
	};

	// InitData struct specialization for Box
	template <>
	struct InitData<Box>
	{
		Vector3 center;
		Vector3 size;
	};
}

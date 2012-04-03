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


DECLARE_CLASS_HANDLE(gfx, BranchNode)	// gfx::BranchNodeHandle


namespace sim
{
	// a cuboid-shaped entity
	class Box : public Entity
	{
	public:
		// types
		struct InitData
		{
			Vector3 center;
			Vector3 size;
		};
		
		// functions
		Box();
		~Box();
		
		// Called from the simulation thread.
		void Init(sim::Engine & simulation_engine, InitData const & init_data);
	private:
		void InitPhysics(sim::Engine & simulation_engine, Vector3 center, Vector3 size);
		void InitGraphics();
		
	public:
		void UpdateModels() const;
	private:
		
		// variables
		gfx::BranchNodeHandle _model;
	};
}

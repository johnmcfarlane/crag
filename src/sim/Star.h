//
//  Star.h
//  crag
//
//  Created by John on 12/22/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Entity.h"

#include "gfx/defs.h"


DECLARE_CLASS_HANDLE(gfx, BranchNode)	// gfx::BranchNodeHandle


namespace sim
{
	// rotating solar light source
	class Star : public Entity
	{
	public:
		// types
		struct InitData
		{
			Scalar radius;
			Scalar year;
		};
		
		// functions
		Star();
		~Star();

		// Called from the simulation thread.
		void Init(sim::Engine & simulation_engine, InitData const & init);
		
		void Tick(sim::Engine & simulation_engine);
		
		void UpdateModels() const;
	
		Scalar GetBoundingRadius() const;

	private:
		gfx::BranchNodeHandle _model;
		Vector3 position;
		Scalar radius;
		Scalar year;
	};
}

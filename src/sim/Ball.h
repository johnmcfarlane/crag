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

#include "gfx/Color.h"
#include "gfx/defs.h"

#include "geom/Sphere.h"


DECLARE_CLASS_HANDLE(gfx, BranchNode)	// gfx::BranchNodeHandle


namespace sim
{
	// a round entity
	class Ball : public Entity
	{
	public:
		// functions
		DECLARE_ALLOCATOR(Ball);

		Ball(Init const & init, Sphere3 const & sphere);
		~Ball();
		
	private:
		void InitPhysics(sim::Engine & simulation_engine, Sphere3 const & sphere);
		void InitGraphics(Sphere3 const & sphere);
		virtual gfx::Color4f GetColor() const;
		
	public:
		void UpdateModels() const;
		
		gfx::BranchNodeHandle const & GetModel() const;

	private:
		// variables
		gfx::BranchNodeHandle _model;
	};
}

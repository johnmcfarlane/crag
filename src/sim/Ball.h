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


namespace gfx
{
	class Ball;
}


namespace sim
{
	// a round entity
	class Ball : public Entity
	{
	public:
		Ball();
		~Ball();
		
		// Called from the simulation thread.
		void Init(Simulation & simulation, InitData<Ball> const & init_data);
	private:
		void InitPhysics(Simulation & simulation, Sphere3 const & sphere);
		void InitGraphics(Sphere3 const & sphere);
		virtual gfx::Color4f GetColor() const;
		
	public:
		void UpdateModels() const;
		
	protected:
		gfx::Uid GetGfxUid() const;

	private:
		// variables
		gfx::Uid _gfx_uid;
	};

	// InitData struct specialization for Ball
	template <>
	struct InitData<Ball>
	{
		Sphere3 sphere;
	};
}

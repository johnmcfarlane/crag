/*
 *  Star.h
 *  Crag
 *
 *  Created by John on 12/22/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once

#include "Entity.h"

#include "gfx/Light.h"


namespace sim
{
	class Star : public Entity
	{
	public:
		Star(float init_radius, float init_year);

		void Tick();
	
		float GetBoundingRadius() const;
		sim::Vector3 const & GetPosition() const;
	
		gfx::Light const & GetLight() const;

	private:
		gfx::Light light;
		float radius;
		float year;
	};
}

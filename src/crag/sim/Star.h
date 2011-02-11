/*
 *  Star.h
 *  Crag
 *
 *  Created by John on 12/22/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
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
		Star(SimulationPtr const & s, Scalar init_radius, Scalar init_year);

		// Type-specific allocation via script.
		static Star * Create(PyObject * args);
		
		void Tick(Universe const & universe);
	
		Scalar GetBoundingRadius() const;
		
		virtual Vector3 const & GetPosition() const;
	
	private:
		gfx::Light light;
		Vector3 position;
		Scalar radius;
		Scalar year;
	};
}

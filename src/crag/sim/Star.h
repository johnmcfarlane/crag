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
		DECLARE_SCRIPT_CLASS(Star, Entity);
		
	public:
		Star();

		// Type-specific allocation via script.
		static void Create(Star & star, PyObject & args);
		
		// Called from the simulation thread.
		bool Init(PyObject & args);
		
		void Tick();
	
		Scalar GetBoundingRadius() const;
		
		virtual Vector3 const & GetPosition() const;
	
	private:
		gfx::Light light;
		Vector3 position;
		Scalar radius;
		Scalar year;
	};
}

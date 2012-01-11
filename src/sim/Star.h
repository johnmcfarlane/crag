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


namespace sim
{
	class Star : public Entity
	{
		DECLARE_SCRIPT_CLASS(Star, Entity);
		
	public:
		Star();
		~Star();

		// Type-specific allocation via script.
		static void Create(Star & star, PyObject & args);
		
		// Called from the simulation thread.
		bool Init(Simulation & simulation, PyObject & args);
		
		void Tick(Simulation & simulation);
		
		void UpdateModels() const;
	
		Scalar GetBoundingRadius() const;

	private:
		gfx::Uid _light_uid;
		Vector3 position;
		Scalar radius;
		Scalar year;
	};
}

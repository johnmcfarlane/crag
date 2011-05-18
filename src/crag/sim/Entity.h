/*
 *  Entity.h
 *  Crag
 *
 *  Created by john on 5/21/09.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"


namespace physics
{
	class Body;
}


namespace sim
{
	class Universe;

	
	// The base class for 'things' that exist in the simulation.
	class Entity
	{
	public:
		Entity();
		virtual ~Entity();
		
		// Type-specific allocation via script.
		static Entity * Create(PyObject * args);
		static void Destroy(Entity & entity);

		// general callbacks
		virtual void Tick();
		virtual void Draw() const;
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		virtual bool GetRenderRange(Ray3 const & camera_ray, double * range, bool wireframe) const;
		
		virtual void SetPosition(Vector3 const & position);
		virtual Vector3 const & GetPosition() const = 0;
		
		virtual physics::Body * GetBody();
		virtual physics::Body const * GetBody() const;

		// Verification
	#if defined(VERIFY)
		void Verify(Entity const & entity);
	#endif
	
		//DUMP_OPERATOR_DECLARATION(Entity);
	};
	
}

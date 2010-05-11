/*
 *  Entity.h
 *  Crag
 *
 *  Created by john on 5/21/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

// TODO: Really shouldn't be included here.
#include <ode/ode.h>


namespace sim
{
	class Entity
	{
	public:
		Entity();
		virtual ~Entity();

		virtual bool IsShadowCatcher() const;

		// general callbacks
		virtual void Tick();
		virtual void Draw() const;
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;

		virtual float GetBoundingRadius() const = 0;
		virtual sim::Vector3 const & GetPosition() const = 0;
		virtual class PhysicalBody * GetPhysicalBody();
		virtual bool CustomCollision(PhysicalBody & that_body) const;

		// Verification
	#if VERIFY
		void Verify(Entity const & entity);
	#endif
	
		//DUMP_OPERATOR_DECLARATION(Entity);
	};
}

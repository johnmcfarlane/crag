//
//  Entity.h
//  crag
//
//  Created by john on 5/21/09.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"


namespace physics
{
	class Body;
}


namespace sim
{
	// The base class for 'things' that exist in the simulation.
	class Entity : public smp::ObjectBase<Entity, sim::Engine>
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef physics::Body Body;

		////////////////////////////////////////////////////////////////////////////////
		// functions
		Entity();
		virtual ~Entity();
		
		// general callbacks
		virtual void Tick(sim::Engine & simulation_engine);
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		
		virtual void UpdateModels() const;

		void SetBody(Body * body);
		Body * GetBody();
		Body const * GetBody() const;
		
		Transformation GetTransformation() const;

		// Verification
	#if defined(VERIFY)
		//virtual void Verify() override;
	#endif

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		Body * _body;
	};
}

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

#include "script/Object.h"


namespace physics
{
	class Body;
}


namespace sim
{
	// forward-declaration
	class Simulation;
	
	
	// The base class for 'things' that exist in the simulation.
	class Entity : public script::Object
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types

		typedef physics::Body Body;

		////////////////////////////////////////////////////////////////////////////////
		// functions
		Entity();
		virtual ~Entity();
		
		Uid GetUid() const;
		
		// general callbacks
		virtual void Tick(Simulation & simulation);
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		
		virtual void UpdateModels() const;

		void SetBody(Body * body);
		Body * GetBody();
		Body const * GetBody() const;

		// Verification
	#if defined(VERIFY)
		void Verify(Entity const & entity);
	#endif

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		Uid _uid;
		Body * _body;
	};
}

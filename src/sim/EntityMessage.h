//
//  EntityMessage.h
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Simulation.h"

#include "Entity.h"

#include "smp/Message.h"

#include "script/MetaClass.h"


namespace sim
{
	// Classes derived from EntityMessage are functors which are
	// called on the correct object with the correct type given a UID.
	// Must be passed to sim::Daemon::SendMessage.
	template <typename DERIVED_TYPE>
	class EntityMessage : public smp::Message<sim::Simulation>
	{
	public:
		// functions
		EntityMessage(sim::Uid uid)
		: _uid(uid)
		{
		}
		
	private:
		virtual void operator() (DERIVED_TYPE & derived) const = 0;
		
		void operator() (sim::Simulation & simulation) const final
		{
			// Find the entity which matches the UID.
			sim::Entity * entity = simulation.GetEntity(_uid);
			if (entity == nullptr)
			{
				// presumably already destroyed
				return;
			}
			
			// The UID passed in MUST represent an entity of derived type, DERIVED_TYPE.
			Assert(script::GetPtr<DERIVED_TYPE>(entity->ob_base) != nullptr);
			
			// Cast entity to the derived type.
			DERIVED_TYPE & derived = static_cast<DERIVED_TYPE &>(* entity);
			
			// Call the functor overload.
			operator() (derived);
		}
		
		// variables
		sim::Uid _uid;
	};
}

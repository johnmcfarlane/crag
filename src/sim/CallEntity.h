//
//  CallEntity.h
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
	// Message sent by CallEntity function. (See below.)
	template <typename ENTITY_TYPE, typename FUNCTOR>
	class EntityMessage : public smp::Message<sim::Simulation>
	{
	public:
		// functions
		EntityMessage(Uid uid, FUNCTOR const & functor)
		: _uid(uid)
		, _functor(functor)
		{
		}
		
		void operator() (sim::Simulation & simulation) const final
		{
			// Find the entity which matches the UID.
			sim::Entity * entity = simulation.GetEntity(_uid);
			
			// TODO: Make this type safe.
			ENTITY_TYPE * derived = static_cast<ENTITY_TYPE *>(entity);
			
			_functor(derived);
		}

	private:
		// variables
		Uid _uid;
		FUNCTOR _functor;
	};
	
	// Thread-safe call to an entity by its UID.
	// Takes functor of type, void f(ENTITY_TYPE * entity).
	template <typename ENTITY_TYPE, typename FUNCTOR>
	void CallEntity(Uid uid, FUNCTOR const const & functor)
	{
		EntityMessage<ENTITY_TYPE, FUNCTOR> message(uid, functor);
		sim::Daemon::SendMessage(message);
	}
}

//
//  EntityHandle.h
//  crag
//
//  Created by John McFarlane on 2/12/12.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/CallEntity.h"


namespace script
{
	// Base class for EntityHandle.
	class EntityHandleBase
	{
	public:
		// functions
		void Destroy();
		
		sim::Uid GetUid() const;
		void SetUid(sim::Uid uid);
		
	private:
		// variables
		sim::Uid _uid;
	};
	
	// A handle to an object of type, ENTITY_TYPE, which is derived from sim::Entity.
	// Currently used by scripts to control sim entities in a thread-safe manner.
	template <typename ENTITY_TYPE>
	class EntityHandle : public EntityHandleBase
	{
		// types
		typedef sim::InitData<ENTITY_TYPE> InitData;
		
	public:
		// functions
		void Create(InitData const & init_data)
		{
			ENTITY_TYPE & entity = ref(new ENTITY_TYPE());
			sim::Daemon::Call< ENTITY_TYPE *, InitData >(& entity, init_data, & sim::Simulation::OnNewEntity);
			SetUid(entity.GetUid());
		}

		template <typename FUNCTOR>
		void Call(FUNCTOR const & functor)
		{
			sim::CallEntity<ENTITY_TYPE, FUNCTOR>(GetUid(), functor);
		}
	};
}

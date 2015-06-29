//
//  entity/sim/Health.h
//  crag
//
//  Created by John McFarlane on 2015-06-29.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <sim/Entity.h>

#include <core/counted_object.h>
#include <core/RosterObjectDeclare.h>

#include "Signal.h"

namespace sim
{
	DECLARE_CLASS_HANDLE(Entity);	// sim::EntityHandle

	// a component belonging to AnimatController which manages a simple health scalar;
	// destroys entity when health hits zero and transmits health value
	class Health final
		: private crag::counted_object<Health>
	{
	public:
		// functions
		CRAG_ROSTER_OBJECT_DECLARE(Health);
		CRAG_VERIFY_INVARIANTS_DECLARE(Health);

		Health(EntityHandle entity_handle) noexcept;

		Transmitter & GetTransmitter() noexcept;

		void IncrementHealth(SignalType delta) noexcept;
	private:
		void Tick() noexcept;
		void OnDeath() noexcept;

		EntityHandle _entity_handle;
		SignalType _amount = 1.f;
		Transmitter _transmitter;
	};
}

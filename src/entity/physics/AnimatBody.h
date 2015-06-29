//
//  entity/physics/AnimatBody.h
//  crag
//
//  Created by John McFarlane on 2015-06-12.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <physics/SphereBody.h>

#include <sim/defs.h>

#include <entity/sim/Signal.h>

namespace physics
{
	// physics for Animat - a sphere with health
	class AnimatBody final
		: public SphereBody
	{
	public:
		// functions
		CRAG_ROSTER_OBJECT_DECLARE(AnimatBody);
		CRAG_VERIFY_INVARIANTS_DECLARE(AnimatBody);

		AnimatBody(Transformation const & transformation, Vector3 const * velocity, physics::Engine & engine,
			Scalar radius, sim::Entity & entity) noexcept;

		void PostTick() noexcept;

		void AddHealthReceiver(sim::Receiver & reciever);

	private:
		void OnContact(Body & that_body) noexcept override;
		bool HasHealth() const noexcept override;
		void OnZeroHealth() const noexcept;

		// variables
		sim::SignalType _health = 1.f;
		sim::Entity & _entity;
		sim::Transmitter _health_transmitter;
	};
}

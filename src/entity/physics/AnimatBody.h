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

namespace sim
{
	class Health;
}

namespace physics
{
	// physics for Animat - spheres which exchange health on contact
	class AnimatBody final
		: public SphereBody
	{
	public:
		// functions
		CRAG_ROSTER_OBJECT_DECLARE(AnimatBody);
		CRAG_VERIFY_INVARIANTS_DECLARE(AnimatBody);

		AnimatBody(Transformation const & transformation, Vector3 const * velocity, physics::Engine & engine,
			Scalar radius, sim::Health & health) noexcept;

	private:
		void OnContact(Body & that_body) noexcept override;
		bool HasHealth() const noexcept override;

		// variables
		sim::Health & _health;
	};
}

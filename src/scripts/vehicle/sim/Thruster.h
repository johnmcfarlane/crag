//
//  sim/Thruster.h
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "sim/defs.h"

namespace gfx { DECLARE_CLASS_HANDLE(Thruster); }	// gfx::ThrusterHandle

namespace physics
{
	class Body;
}

namespace sim
{
	struct Thruster
	{
	public:
		Thruster(Ray3 const & ray, gfx::ThrusterHandle const & model);

		float GetThrustFactor() const;
		void SetThrustFactor(float thrust_factor);

		// adds a puff of smoke
		void UpdateModel() const;

		// Add force from an individual Thruster to the vehicle's body.
		void ApplyThrust(physics::Body & body) const;

#if defined(VERIFY)
		void Verify() const;
#endif

	private:
		Ray3 const _ray;	// position/direction of Thruster relative to vehicle
		gfx::ThrusterHandle const _model;

		float _thrust_factor;
	};
		
	typedef std::shared_ptr<Thruster> ThrusterPtr;
}

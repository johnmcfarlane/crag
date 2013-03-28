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

namespace gfx 
{ 
	DECLARE_CLASS_HANDLE(Object);	// gfx::ObjectHandle
	DECLARE_CLASS_HANDLE(Thruster);	// gfx::ThrusterHandle
}

namespace physics
{
	class Body;
}

namespace sim
{
	// applies force to an entity; is owned by a controller
	struct Thruster
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		Thruster(Entity & entity, Ray3 const & ray, bool light);
		virtual ~Thruster();

#if defined(VERIFY)
		void Verify() const;
#endif

		void SetParentModel(gfx::ObjectHandle parent_model);

		Entity & GetEntity();

		// get/set the amount of thrust being applied to entity's body
		float GetThrustFactor() const;
		void SetThrustFactor(float thrust_factor);

		// adds a puff of smoke
		void UpdateModel() const;

		void Tick();
	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables

		sim::Entity & _entity;
		Ray3 const _ray;	// position/direction of Thruster relative to vehicle
		gfx::ThrusterHandle _model;
		float _thrust_factor;
	};
}

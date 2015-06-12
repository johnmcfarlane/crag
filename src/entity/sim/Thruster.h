//
//  sim/Thruster.h
//  crag
//
//  Created by John McFarlane on 11/9/11.
//	Copyright 2011 John McFarlane. All rights reserved.
//	This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Signal.h"

#include "sim/defs.h"

#include "core/counted_object.h"
#include "core/RosterObjectDeclare.h"

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
	// applies force to an entity; is owned by a controller;
	// ray is in entity-local coordinate space
	class Thruster final
	: public Receiver
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		OBJECT_NO_COPY(Thruster);

		Thruster(Entity & entity, Ray3 const & ray, bool graphical, Scalar thrust_factor);
		virtual ~Thruster();

		CRAG_ROSTER_OBJECT_DECLARE(Thruster);
		CRAG_VERIFY_INVARIANTS_DECLARE(Thruster);

		void SetParentModel(gfx::ObjectHandle parent_handle);

		Entity & GetEntity();
		Entity const & GetEntity() const;

		Ray3 const & GetRay() const;
		void SetRay(Ray3 const & ray);

		void Tick();
	private:
		// adds a puff of smoke
		void UpdateModel();

		////////////////////////////////////////////////////////////////////////////////
		// variables

		sim::Entity & _entity;
		Ray3 _ray;	// position/direction of Thruster relative to vehicle
		gfx::ThrusterHandle _model;
	};
}

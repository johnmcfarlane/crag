/*
 *  Entity.h
 *  Crag
 *
 *  Created by john on 5/21/09.
 *  Copyright 2009-2011 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#include "script/Object.h"

#include "smp/Uid.h"

#include "core/intrusive_list.h"


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
		DECLARE_SCRIPT_CLASS(Entity, script::Object);
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		typedef core::intrusive::hook<Entity> HookType;
		HookType _hook;
	public:
		DEFINE_TEMPLATED_INTRUSIVE_LIST_TYPE(Entity, _hook, List);
		typedef List::const_iterator const_iterator;
		typedef List::iterator iterator;

		typedef smp::Uid Uid;
		typedef physics::Body Body;

		////////////////////////////////////////////////////////////////////////////////
		// functions
		Entity();
		virtual ~Entity();
		
		Uid GetUid() const;
		
		// Type-specific allocation via script. DO NOT CALL.
		static void Create(Entity & entity, PyObject & args);

		// Type-specific allocation via script.
		static void Destroy(Entity & entity);
		
		// Called from the simulation thread.
		virtual bool Init(Simulation & simulation, PyObject & args) = 0;

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

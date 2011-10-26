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
		
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		typedef physics::Body Body;

		// functions
		Entity();
		virtual ~Entity();
		
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

		Body * _body;
	};
	
	
	// Entity-related messages
	struct AddEntityMessage
	{
		Entity & entity;
		PyObject & args;
	};
	
	struct RemoveEntityMessage
	{
		Entity & entity;
	};
	
}
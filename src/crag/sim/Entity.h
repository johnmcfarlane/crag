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


namespace gfx
{
	class Scene;
}


namespace sim
{
	
	// The base class for 'things' that exist in the simulation.
	class Entity : public script::Object
	{
		DECLARE_SCRIPT_CLASS(Entity, script::Object);
		
	public:
		Entity();
		virtual ~Entity();
		
		// Type-specific allocation via script.
		static void Destroy(Entity & entity);
		
		// Called from the simulation thread.
		virtual bool Init(PyObject & args) = 0;

		// general callbacks
		virtual void Tick();
		virtual void Draw(gfx::Scene const & scene) const;
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		virtual bool GetRenderRange(Ray3 const & camera_ray, double * range, bool wireframe) const;
		
		virtual void SetPosition(Vector3 const & position);
		virtual Vector3 const & GetPosition() const = 0;

		// Verification
	#if defined(VERIFY)
		void Verify(Entity const & entity);
	#endif
	
		//DUMP_OPERATOR_DECLARATION(Entity);
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

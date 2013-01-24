//
//  Entity.h
//  crag
//
//  Created by john on 5/21/09.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"


namespace physics
{
	class Body;
}

namespace gfx
{
	DECLARE_CLASS_HANDLE(BranchNode);	// gfx::BranchNodeHandle
}

namespace sim
{
	// The base class for 'things' that exist in the simulation.
	class Entity : public smp::Object<Entity, sim::Engine>
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef smp::Object<Entity, sim::Engine> super;
	public:
		typedef physics::Body Body;

		////////////////////////////////////////////////////////////////////////////////
		// functions

		DECLARE_ALLOCATOR(Vehicle);

		Entity(Init const & init);
		virtual ~Entity();
		
		// general callbacks
		virtual void Tick();
		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;
		
		// physics
		void SetBody(Body * body);
		Body * GetBody();
		Body const * GetBody() const;
		Transformation GetTransformation() const;

		// graphics
		gfx::BranchNodeHandle GetModel() const;
		void SetModel(gfx::BranchNodeHandle model);
		virtual void UpdateModels() const;

		// Verification
	#if defined(VERIFY)
		virtual void Verify() const override;
	#endif

	private:
		////////////////////////////////////////////////////////////////////////////////
		// variables
		Body * _body;
		gfx::BranchNodeHandle _model;
	};
}

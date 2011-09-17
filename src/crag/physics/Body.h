/*
 *  Body.h
 *  Crag
 *
 *  Created by John on 10/29/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "defs.h"

#include "form/defs.h"


namespace physics
{
	// forward-declarations
	class Engine;
	class IntersectionFunctor;
	
	// Body wraps ODE geometry and physical body and handles certain collisions.
	class Body
	{
	protected:
		Body(Engine & engine, dGeomID init_geom_id, bool movable);
	public:
		virtual ~Body();
		
		dGeomID GetGeomId() const;
		virtual void SetDensity(Scalar density) = 0;
		Scalar GetMass() const;	// -ve means infinite
		
		Vector3 const & GetPosition() const;
		void SetPosition(Vector3 const &) const;		
		void GetRotation(Matrix4 & rot) const;
		void SetRotation(Matrix4 const & matrix);
		
		bool IsMovable() const;
		void SetLinearDamping(Scalar linear_damping);
		void SetAngularDamping(Scalar angular_damping);
		void AddRelTorque(Vector3 const & torque);
		void AddForce(Vector3 const & force);
		void AddRelForce(Vector3 const & force);
		void AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos);
		
		virtual bool OnCollision(Engine & engine, Body const & that_body) const;
		
		virtual void OnDeferredCollisionWithPlanet(Body const & body, IntersectionFunctor & functor) const;
		virtual void OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctor & functor) const;
		
	protected:
		dGeomID geom_id;	// the collision info
		dBodyID body_id;	// the dynaical info
	};
}

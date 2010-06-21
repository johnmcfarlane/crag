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


namespace sim
{
	class Entity;
}


namespace physics
{
	class SphericalBody;
	class Singleton;

	class Body
	{
		friend class ::physics::Singleton;
		
	protected:
		Body(sim::Entity & entity, dBodyID body_id, dGeomID geom_id);
		virtual ~Body();
	public:
				
		virtual void SetDensity(Scalar density) = 0;

		Vector3 const & GetPosition() const;
		void SetPosition(Vector3 const &) const;		
		void GetRotation(Matrix4 & rot) const;
		void SetRotation(Matrix4 const & matrix);
		
		bool IsMovable() const;
		void SetLinearDamping(Scalar linear_damping);
		void SetAngularDamping(Scalar angular_damping);
		void AddRelTorque(Vector3 const & torque);
		void AddRelForce(Vector3 const & force);
		void AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos);
		
		virtual bool OnCollision(Body & that_body) = 0;
		virtual bool OnCollisionWithSphericalBody(SphericalBody & that_sphere);
	private:
		
		// Data
		sim::Entity & entity;

		// ODE objects
	protected:
		dBodyID body_id;
		dGeomID geom_id;
	};
	
}
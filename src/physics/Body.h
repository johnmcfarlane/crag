//
//  Body.h
//  crag
//
//  Created by John on 10/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Location.h"

namespace core
{
	namespace locality
	{
		class Roster;
	}
}

namespace physics
{
	// forward-declarations
	class Body;
	class Engine;
	class IntersectionFunctor;
	
	void Attach(dJointID joint, Body const & body1, Body const & body2);
	bool IsAttached(Body const & body1, Body const & body2);
	
	// Body wraps ODE geometry and physical body and handles certain collisions.
	class Body : public Location
	{
	public:
		Body(Transformation const & transformation, Vector3 const * velocity, Engine & engine, dGeomID init_geom_id);
		virtual ~Body() = 0;
		
		Body * GetBody() final;
		Body const * GetBody() const final;

		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;

		dGeomID GetGeomId() const;
		virtual void SetDensity(Scalar density) = 0;
		Scalar GetMass() const;	// -ve means infinite
		
		void SetTransformation(Transformation const & transformation) final;
		
		Vector3 GetRelativePointVelocity(Vector3 const & point) const;
		Vector3 GetVelocity() const;
		
		bool GetIsCollidable() const;
		void SetIsCollidable(bool collidable);
		
		bool IsMovable() const;
		void SetLinearDamping(Scalar linear_damping);
		void SetAngularDamping(Scalar angular_damping);
		void AddRelTorque(Vector3 const & torque);
		void AddForce(Vector3 const & force);
		void AddRelForce(Vector3 const & force);
		void AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos);
		
		virtual bool OnCollision(Engine & engine, Body const & that_body) const;
		
		virtual void OnDeferredCollisionWithBox(Body const & body, IntersectionFunctor & functor) const;
		virtual void OnDeferredCollisionWithPlanet(Body const & body, IntersectionFunctor & functor) const;
		virtual void OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctor & functor) const;
		
		friend void Attach(dJointID joint, Body const & body1, Body const & body2);
		friend bool IsAttached(Body const & body1, Body const & body2);
		
#if defined(VERIFY)
		virtual void Verify() const;
#endif
	private:
		Vector3 const & GetGeomTranslation() const;
		void SetGeomTranslation(Vector3 const & translation);

		Matrix33 const & GetGeomRotation() const;
		void SetGeomRotation(Matrix33 const & matrix);

		Transformation GetGeomTransformation() const;
		void SetGeomTransformation(Transformation const & transformation);

		void Tick();

	protected:
		dGeomID geom_id;	// the collision info
		dBodyID body_id;	// the dynaical info
		core::locality::Roster & _roster;
	};
}

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

namespace physics
{
	// forward-declarations
	class Body;
	class Engine;
	class IntersectionFunctor;
	
	void Attach(JointHandle joint, Body const & body1, Body const & body2);
	bool IsAttached(Body const & body1, Body const & body2);
	
	// Body wraps ODE geometry and physical body and handles certain collisions.
	class Body : public Location
	{
	protected:
		Body(Engine & engine, CollisionHandle collision_handle, bool movable);
	public:
		virtual ~Body() override;
		
		virtual Body * GetBody() final;
		virtual Body const * GetBody() const final;

		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;

		CollisionHandle GetCollisionHandle() const;
		virtual Vector3 GetScale() const = 0;
		virtual void SetDensity(Scalar density) = 0;
		Scalar GetMass() const;	// -ve means infinite
		
		virtual Vector3 GetPosition() const final;
		void SetPosition(Vector3 const &) const;
		
		Vector3 GetRelativePointVelocity(Vector3 const & point) const;
		Vector3 GetVelocity() const;
		
		virtual Matrix33 GetRotation() const final;
		void SetRotation(Matrix33 const & matrix);

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
		virtual void OnDeferredCollisionWithRay(Body const & body, IntersectionFunctor & functor) const;
		virtual void OnDeferredCollisionWithSphere(Body const & body, IntersectionFunctor & functor) const;
		
		friend void Attach(JointHandle joint, Body const & body1, Body const & body2);
		friend bool IsAttached(Body const & body1, Body const & body2);
		
#if defined(VERIFY)
		virtual void Verify() const;
#endif

	protected:
		CollisionHandle _collision_handle;	// the collision info
		BodyHandle _body_handle;	// the dynaical info
	};
}

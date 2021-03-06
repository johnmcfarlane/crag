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
	
	void Attach(JointHandle joint, Body const & body1, Body const & body2);
	bool IsAttached(Body const & body1, Body const & body2);
	
	// Body wraps ODE geometry and physical body and handles certain collisions.
	class Body : public Location
	{
	public:
		CRAG_ROSTER_OBJECT_DECLARE(Body);

		Body(Transformation const & transformation, Vector3 const * velocity, Engine & engine, CollisionHandle collision_handle);
		virtual ~Body();

		void PreTick();
		void PostTick();

		virtual bool ObeysGravity() const;
		
		// the sum force being exherted on this body as a result of gravity
		void SetGravitationalForce(Vector3 const & gravitational_force);
		Vector3 const & GetGravitationalForce() const;

		BodyHandle GetBodyHandle() const;
		CollisionHandle GetCollisionHandle() const;
		virtual void SetDensity(Scalar density);
		
		Scalar GetMass() const;	// -ve means infinite
		virtual void SetMass(Mass const & mass) const;
		
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
		void AddTorque(Vector3 const & torque);
		void AddForceAtPos(Vector3 const & force, Vector3 const & pos);
		void AddForceAtRelPos(Vector3 const & force, Vector3 const & pos);
		void AddRelForce(Vector3 const & force);
		void AddRelForceAtPos(Vector3 const & force, Vector3 const & pos);
		void AddRelForceAtRelPos(Vector3 const & force, Vector3 const & pos);
		
		void SetIsCollidable(Body const & body, bool collidable);
		bool IsCollidable(Body const & body) const;
		
		virtual bool HandleCollision(Body & that_body, ContactFunction & contact_function) = 0;
		virtual bool HandleCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere, ContactFunction & contact_function);
		virtual bool HandleCollisionWithRay(Body & body);

		virtual void OnContact(Body & that_body);

		virtual void DebugDraw() const;

		friend void Attach(JointHandle joint, Body const & body1, Body const & body2);
		friend bool IsAttached(Body const & body1, Body const & body2);

		virtual bool HasHealth() const;

		CRAG_VERIFY_INVARIANTS_DECLARE(Body);
	private:
		virtual Vector3 const & GetGeomTranslation() const;
		virtual void SetGeomTranslation(Vector3 const & translation);

		virtual Matrix33 const & GetGeomRotation() const;
		virtual void SetGeomRotation(Matrix33 const & matrix);

	protected:
		Transformation GetGeomTransformation() const;
		void SetGeomTransformation(Transformation const & transformation);

		// variables
		Engine & _engine;
		CollisionHandle _collision_handle;	// the collision info
		BodyHandle _body_handle;	// the dynaical info
		
	private:
		Body const * _exception;
		Vector3 _gravitational_force;
	};
}

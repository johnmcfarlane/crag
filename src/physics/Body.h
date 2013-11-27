//
//  Body.h
//  crag
//
//  Created by John on 10/29/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ContactInterface.h"
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
		Body(Transformation const & transformation, Vector3 const * velocity, Engine & engine, CollisionHandle collision_handle);
		virtual ~Body() = 0;
		
		Body * GetBody() final;
		Body const * GetBody() const final;

		virtual void GetGravitationalForce(Vector3 const & pos, Vector3 & gravity) const;

		BodyHandle GetBodyHandle() const;
		CollisionHandle GetCollisionHandle() const;
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
		
		void SetIsCollidable(Body const & body, bool collidable);
		bool IsCollidable(Body const & body) const;
		
		virtual bool OnCollision(Body & that_body, ContactInterface & contact_interface) = 0;
		virtual bool OnCollisionWithSolid(Body & body, Sphere3 const & bounding_sphere, ContactInterface & contact_interface);
		virtual bool OnCollisionWithRay(Body & body);

		friend void Attach(JointHandle joint, Body const & body1, Body const & body2);
		friend bool IsAttached(Body const & body1, Body const & body2);
		
		CRAG_VERIFY_INVARIANTS_DECLARE(Body);
	private:
		Vector3 const & GetGeomTranslation() const;
		void SetGeomTranslation(Vector3 const & translation);

		Matrix33 const & GetGeomRotation() const;
		void SetGeomRotation(Matrix33 const & matrix);

		Transformation GetGeomTransformation() const;
		void SetGeomTransformation(Transformation const & transformation);

		void Tick();

	protected:
		Engine & _engine;
		
	private:
		CollisionHandle _collision_handle;	// the collision info
		BodyHandle _body_handle;	// the dynaical info
		Body const * _exception;
	};
}

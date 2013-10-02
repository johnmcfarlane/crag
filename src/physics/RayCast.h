//
//  Ray.h
//  crag
//
//  Created by John on 2013-04-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Body.h"

#include "form/RayCastResult.h"

namespace physics
{
	// Represents a ray to the physics system; has collision by is not dynamic.
	class RayCast : public Body
	{
	public:
		RayCast(Engine & engine, Scalar length);
		~RayCast();
		
#if defined(VERIFY)
		void Verify() const final;
#endif

		void SetDirection(Vector3 const & direction);
		Vector3 GetDirection() const;
		
		// ray must have unit direction
		void SetRay(Ray3 ray);
		Ray3 GetRay() const;
		
		void SetLength(Scalar length);
		Scalar GetLength() const;
		
		form::RayCastResult const & GetResult() const;
		void SampleResult(form::RayCastResult const & result);

	private:
		void ResetResult();
		
		void SetDensity(Scalar density) override final;

		bool OnCollision(Body & that_body, ContactInterface & contact_interface) final;
		
		form::RayCastResult _result;
	};
}

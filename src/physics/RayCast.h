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

		CRAG_ROSTER_OBJECT_DECLARE(RayCast);
		CRAG_VERIFY_INVARIANTS_DECLARE(RayCast);

		void SetDirection(Vector3 const & direction);
		Vector3 GetDirection() const;
		
		// ray must have unit direction
		void SetRay(Ray3 const & ray);
		Ray3 GetRay() const;
		
		void SetLength(Scalar length);
		Scalar GetLength() const;
		
		form::RayCastResult const & GetResult() const;
		void SampleResult(form::RayCastResult const & result);

		void DebugDraw() const override;

	private:
		void ResetResult();
		
		void SetDensity(Scalar density) override;

		bool HandleCollision(Body & that_body, ContactFunction & contact_function) override;

#if defined(CRAG_DEBUG)
		void OnContact(Body const & that_body) final;
#endif

		form::RayCastResult _result;
	};
}

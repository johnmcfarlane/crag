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
		
		bool IsContacted() const;

		Scalar GetContactDistance() const;
		void SampleContact(Scalar contact_distance);

	private:
		void ResetContactDistance();
		
		void SetDensity(Scalar density) override final;

		bool OnCollision(Body & that_body) final;
		
		Scalar _contact_distance;
		static constexpr Scalar max_contact_distance = std::numeric_limits<Scalar>::max();
	};
}

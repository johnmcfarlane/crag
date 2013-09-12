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
	// TODO: Change interface so that Ray3 has normalized direction
	class RayCast : public Body
	{
	public:
		RayCast(Engine & engine);
		~RayCast();
		
		void SetDirection(Vector3 const & direction);
		Vector3 GetDirection() const;
		
		// ray's direction includes its length, i.e. it is not unit length
		void SetRay(Ray3 ray);
		Ray3 GetRay() const;

		Scalar GetLength() const;
		Scalar GetPenetrationDepth() const;

		void SetSample(float depth) const;
		void ResetSample();
	private:
		virtual void SetDensity(Scalar density) override final;

		virtual bool OnCollision(Body const & that_body) const final;
		
#if defined(VERIFY)
		virtual void Verify() const override final;
#endif

		float _penetration_depth;
		mutable float _min_sample;
	};
}

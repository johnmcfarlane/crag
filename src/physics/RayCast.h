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
	// Represents a ray to the physics system; has collision by is not dynamic
	class RayCast : public Body
	{
	public:
#if defined(VERIFY)
		virtual void Verify() const override final;
#endif

		RayCast(Engine & engine);
		
		void setDirection(Vector3 const & direction);
		Vector3 getDirection() const;
		
		// ray's direction includes its length, i.e. it is not unit length
		void setRay(Ray3 ray);
		Ray3 getRay() const;

		Scalar GetLength() const;

	private:
		virtual Vector3 GetScale() const override final;
		virtual void SetDensity(Scalar density) override final;

		virtual void OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctor & functor) const;
	};
}

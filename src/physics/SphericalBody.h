//
//  SphericalBody.h
//  crag
//
//  Created by John on 6/16/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Body.h"


namespace physics
{
	class Engine;

	// TODO: Rename this to Sphere and give it a comment.
	// TODO: Repeat for BoxBody and PlanetaryBody.
	class SphericalBody : public Body
	{
	public:
		SphericalBody(Transformation const & transformation, Vector3 const * velocity, physics::Engine & engine, Scalar radius);
		
		void SetRadius(Scalar radius) const;
		Scalar GetRadius() const;
		
		virtual void SetDensity(Scalar density) override;
		
	protected:
		virtual void OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctorRef const & functor) const;
	};
	
}

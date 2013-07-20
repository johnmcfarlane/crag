//
//  BoxBody.h
//  crag
//
//  Created by John McFarlane on 9/5/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Body.h"


namespace physics
{
	class Engine;
	
	class BoxBody : public Body
	{
	public:
		BoxBody(Transformation const & transformation, Vector3 const * velocity, Engine & engine, Vector3 const & dimensions);
		
		void SetDimensions(Vector3 const & dimensions) const;
		Vector3 GetDimensions() const;
		
		virtual void SetDensity(Scalar density);
		
	protected:
		virtual void OnDeferredCollisionWithPlanet(Body const & planet, IntersectionFunctor & functor) const;
	};
	
}

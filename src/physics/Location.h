//
//  Location.h
//  crag
//
//  Created by John on 2013/01/24.
//  Copyright 2009-2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "defs.h"

#include "geom/Transformation.h"

#include "core/counted_object.h"

namespace physics
{
	// base class of the component representing an entity's physical location
	class Location : private crag::counted_object<Location>
	{
		OBJECT_NO_COPY(Location);
	public:
		Location(Transformation const & transformation);
		virtual ~Location();

		Vector3 GetTranslation() const;
		Matrix33 const & GetRotation() const;

		Transformation const & GetTransformation() const;
		virtual void SetTransformation(Transformation const & transformation) = 0;

		// the amount of attractive influence this body has at a given point in space
		virtual Vector3 GetGravitationalAttraction(Vector3 const & pos) const;
		virtual bool ObeysGravity() const;

		CRAG_VERIFY_INVARIANTS_DECLARE(Location);
		
		static Location const & null;
	private:
		Transformation _transformation;
	};
}

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

namespace physics
{
	class Body;

	// A locator which gets its location from a physics::Body
	class Location
	{
		OBJECT_NO_COPY(Location);
	public:
		Location(Transformation const & transformation);
		virtual ~Location();

		virtual Body * GetBody();
		virtual Body const * GetBody() const;

		Vector3 GetTranslation() const;
		Matrix33 const & GetRotation() const;

		Transformation const & GetTransformation() const;
		virtual void SetTransformation(Transformation const & transformation) = 0;

		Vector3 Transform(Vector3 local_position) const;
		Vector3 Rotate(Vector3 local_direction) const;

		CRAG_VERIFY_INVARIANTS_DECLARE(Location);
	private:
		Transformation _transformation;
	};
}

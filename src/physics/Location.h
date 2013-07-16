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

namespace physics
{
	class Body;

	// A locator which gets its location from a physics::Body
	class Location
	{
		OBJECT_NO_COPY(Location);
	public:
		Location();
		virtual ~Location();

		virtual Body * GetBody();
		virtual Body const * GetBody() const;

		virtual Vector3 GetScale() const;
		virtual Vector3 GetTranslation() const = 0;
		virtual Matrix33 GetRotation() const;
		virtual Transformation GetTransformation() const;

		virtual Vector3 Transform(Vector3 local) const;

#if defined(VERIFY)
		virtual void Verify() const = 0;
#endif
	};
}

//
//  Location.h
//  crag
//
//  Created by John on 2013/01/24.
//  Copyright 2009-2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "defs.h"

#include "Location.h"

#include "geom/Transformation.h"

namespace physics
{
	// A locator which gets its location from a physics::Body
	class FixedLocation : public Location
	{
	public:
		FixedLocation(Transformation const & transformation);

		virtual Vector3 GetTranslation() const final;
		virtual Matrix33 GetRotation() const final;

#if defined(VERIFY)
		virtual void Verify() const final;
#endif
	private:
		Transformation _transformation;
	};
}

//
//  PassiveLocation.h
//  crag
//
//  Created by John on 2013/01/24.
//  Copyright 2009-2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "Location.h"

namespace physics
{
	// A locator which gets its location from a physics::Body
	class PassiveLocation final : public Location
	{
	public:
		PassiveLocation(Transformation const & transformation);
	private:
		void SetTransformation(Transformation const & transformation) final;
	};
}

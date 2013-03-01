//
//  scripts/ga/AnimatController.h
//  crag
//
//  Created by John McFarlane on 2013-03-01.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "sim/Controller.h"

namespace sim
{
	// governs the behevior of a sim::Entity which is an artificial animal
	class AnimatController : public Controller
	{
	public:
		AnimatController(Entity & entity);

		void Tick();
	};
}

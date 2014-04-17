//
//  ObserverInput.h
//  crag
//
//  Created by John on 1/1/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace sim
{
	// stores a value for each of the six degrees of freedom
	// to be translated into movement
	struct ObserverInput : public std::array<geom::Vector3f, 2>
	{
		enum Index
		{
			translation,
			rotation,
			size
		};

		typedef std::array<geom::Vector3f, size> super;

		ObserverInput();
	};

	// translate keyboard state into inputs for ObserverController
	ObserverInput GetObserverInput();
}

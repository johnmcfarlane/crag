//
//  ssga.h
//  crag
//
//  Created by John McFarlane on 2015-07-05.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <geom/Space.h>

namespace sim
{
	class Engine;
}

namespace ssga
{
	// top-level functions of a steady-state genetic algorithm
	void Init(sim::Engine & engine, geom::uni::Vector3 const & spawn_pos) noexcept;
	void Deinit(sim::Engine & engine) noexcept;
	void Tick(sim::Engine & engine) noexcept;
}

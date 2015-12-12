//
//  core/fixed_point.h
//  crag
//
//  Created by John McFarlane on 2015-07-08.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <sg14/fixed_point>

namespace crag
{
	namespace core
	{
		using namespace sg14;
	}
}

namespace std
{
	// somewhat prematurely promoted to std in order to match signature of existing std::* overloads
	using sg14::sqrt;
	using sg14::abs;
}

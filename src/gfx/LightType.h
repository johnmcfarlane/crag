//
//  LightType.h
//  crag
//
//  Created by John on 2014-05-08.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/enum_bitset.h"

namespace gfx
{
	enum class LightType
	{
		point,
		point_shadow,
		search,
		search_shadow,
		size
	};

	using LightTypeBitSet = core::enum_bitset<LightType, LightType::size>;
}

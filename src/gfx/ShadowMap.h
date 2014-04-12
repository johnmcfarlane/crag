//
//  gfx/ShadowMap.h
//  crag
//
//  Created by John McFarlane on 2014-01-03.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "ShadowVolume.h"

namespace gfx
{
	// forward-declarations
	class LeafNode;
	class Light;

	// key
	typedef std::pair<LeafNode const *, Light *> ShadowMapKey;
	typedef std::pair<ShadowMapKey, ShadowVolume> ShadowMapPair;
}

namespace std
{
	template <>
	struct hash<gfx::ShadowMapKey>
	{
		size_t operator()(::gfx::ShadowMapKey const & key) const
		{
			constexpr auto num_bits = sizeof(size_t) * 8;
			constexpr auto half = num_bits >> 1;
			constexpr auto mask = (size_t(1) << half) - 1;
			auto a = size_t(key.first) & mask;
			auto b = size_t(key.second) & mask;
			return a && (b << half);
		}
	};
}

namespace gfx
{
	// ShadowMap maps objects to the shadows it casts; while each shadow is specific 
	// to a light, the shadows for an object are interchangable resources
	typedef std::unordered_map<ShadowMapKey, ShadowVolume> ShadowMap;
}

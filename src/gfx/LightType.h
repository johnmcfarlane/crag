//
//  LightType.h
//  crag
//
//  Created by John on 2014-05-08.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/function_ref.h"

namespace gfx
{
	// forward-declaration
	class Light;
	
	// types
	enum class LightResolution
	{
		// is calculated per-vertex; low resolution
		vertex,

		// is calculated per-fragment; high resolution
		fragment,
		
		size
	};
	
	enum class LightType
	{
		// radiates equally in all directions from a single point
		point,
		
		// a directed cone of light
		search,
		
		size
	};

	struct LightAttributes
	{
		// variables
		LightResolution resolution;
		LightType type;
		bool makes_shadow;
	};
	
	using LightFilter = core::function_ref<bool (Light const &)>;
}

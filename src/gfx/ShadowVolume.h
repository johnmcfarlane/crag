//
//  gfx/ShadowVolume.h
//  crag
//
//  Created by John McFarlane on 2014-01-02.
//  Copyright 2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "NonIndexedVboResource.h"
#include "PlainVertex.h"

namespace crag
{
	namespace core
	{
		class Resource;
	}
}

namespace gfx
{
	// ShadowVolume class definition
	typedef gfx::NonIndexedVboResource<PlainVertex, GL_DYNAMIC_DRAW> ShadowVolume;
	using ShadowVolumeResource = crag::core::Resource;
	using ShadowVolumeHandle = crag::core::ResourceHandle<ShadowVolume>;
}

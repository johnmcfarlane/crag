//
//  gfx/RegisterResources.h
//  crag
//
//  Created by John McFarlane on 2012-01-24.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace crag
{
	namespace core
	{
		class ResourceManager;
	}
}

namespace gfx
{
	void RegisterResources(crag::core::ResourceManager & resource_manager);
}

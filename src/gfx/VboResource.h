//
//  VboResource.h
//  crag
//
//  Created by John McFarlane on 2012-01-25.
//  This program is distributed under the terms of the GNU General Public License.
//  Copyright 2012 John McFarlane. All rights reserved.
//

#pragma once

#include "core/counted_object.h"

namespace gfx
{
	// Base class for a graphical resource such as a vertex buffer object
	// which is costly to bind / unbind during rendering.
	class VboResource : private crag::counted_object<VboResource>
	{
		OBJECT_NO_COPY(VboResource);
	public:
		VboResource() = default;

		virtual ~VboResource() { }
		virtual void Activate() const = 0;
		virtual void Deactivate() const = 0;
		virtual void Draw() const = 0;
	};
}

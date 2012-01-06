//
//  Layer.cpp
//  crag
//
//  Created by John McFarlane on 2011-11-22.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Layer.h"

using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// Layer namespace definitions

#if defined(VERIFY)

void Layer::Verify(type layer)
{
	Assert(layer >= Layer::begin);
	Assert(layer < Layer::end);
}

#endif

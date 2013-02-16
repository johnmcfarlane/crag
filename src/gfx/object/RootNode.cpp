//
//  RootNode.cpp
//  crag
//
//  Created by John McFarlane on 2011/11/21.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "RootNode.h"

using namespace gfx;


////////////////////////////////////////////////////////////////////////////////
// gfx::RootNode definitions

RootNode::RootNode(super::Init const & init, Transformation const & transformation)
: Object(init, transformation)
{
}

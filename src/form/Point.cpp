//
//  Point.cpp
//  crag
//
//  Created by John on 5/29/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Point.h"

using namespace form;

////////////////////////////////////////////////////////////////////////////////
// form::Point member definitions

Point::Point()
: pos(Vector3::Zero())
, vert(nullptr)
{
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Point, self)
	CRAG_VERIFY(self.pos);
	CRAG_VERIFY(self.vert);
CRAG_VERIFY_INVARIANTS_DEFINE_END

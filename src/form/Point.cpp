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

#if defined(VERIFY)
void Point::Verify() const
{
	VerifyObject(pos);
	VerifyObjectPtr(vert);
}
#endif

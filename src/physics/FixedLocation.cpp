//
//  FixedLocation.cpp
//  crag
//
//  Created by John on 2013/01/29.
//  Copyright 2009-2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "FixedLocation.h"

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::FixedLocation member definitions

FixedLocation::FixedLocation(Transformation const & transformation) 
: _transformation(transformation)
{ 
}

Vector3 FixedLocation::GetTranslation() const
{
	return _transformation.GetTranslation();
}

Matrix33 FixedLocation::GetRotation() const
{
	return _transformation.GetRotation();
}

#if defined(VERIFY)
void FixedLocation::Verify() const
{
	VerifyObject(_transformation);
}
#endif

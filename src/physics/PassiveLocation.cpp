//
//  PassiveLocation.cpp
//  crag
//
//  Created by John on 2013/01/29.
//  Copyright 2009-2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "PassiveLocation.h"

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// physics::PassiveLocation member definitions

PassiveLocation::PassiveLocation(Transformation const & transformation) 
: Location(transformation)
{ 
}

void PassiveLocation::SetTransformation(Transformation const & transformation)
{
	Location::SetTransformation(transformation);
}

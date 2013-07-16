//
//  Location.cpp
//  crag
//
//  Created by John on 2013/01/29.
//  Copyright 2009-2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Location.h"

#include "geom/Matrix33.h"
#include "geom/Transformation.h"

using namespace physics;

////////////////////////////////////////////////////////////////////////////////
// Location member definitions

Location::~Location() 
{ 
}

Location::Location() 
{ 
}

Body * Location::GetBody()
{
	return nullptr;
}

Body const * Location::GetBody() const
{
	return nullptr;
}

Matrix33 Location::GetRotation() const
{
	static Matrix33 identity = Matrix33::Identity();
	return identity;
}

Transformation Location::GetTransformation() const
{
	return Transformation(GetTranslation(), GetRotation());
}

Vector3 Location::Transform(Vector3 local) const
{
	return GetTransformation().Transform(local);
}

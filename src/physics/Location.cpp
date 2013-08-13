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

Location::Location(Transformation const & transformation) 
: _transformation(transformation)
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

Vector3 Location::GetTranslation() const
{
	return _transformation.GetTranslation();
}

Matrix33 const & Location::GetRotation() const
{
	return _transformation.GetRotation();
}

Transformation const & Location::GetTransformation() const
{
	return _transformation;
}

void Location::SetTransformation(Transformation const & transformation)
{
	_transformation = transformation;
}

Vector3 Location::Transform(Vector3 local_position) const
{
	return _transformation.Transform(local_position);
}

Vector3 Location::Rotate(Vector3 local_rotation) const
{
	return _transformation.Rotate(local_rotation);
}

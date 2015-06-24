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

namespace
{
	class NullLocation : public Location
	{
	};
}

////////////////////////////////////////////////////////////////////////////////
// Location member definitions

Location::~Location() 
{ 
}

Location::Location(Transformation const & transformation)
: _transformation(transformation)
{
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

Vector3 Location::GetGravitationalAttraction(Vector3 const &) const
{
	return Vector3::Zero();
}

bool Location::ObeysGravity() const
{
	return false;
}

CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Location, self)
	CRAG_VERIFY(self._transformation);
CRAG_VERIFY_INVARIANTS_DEFINE_END

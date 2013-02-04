//
//  origin.h
//  crag
//
//  Created by John on 2012-12-23.
//  Copyright 2009-2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Matrix33.h"
#include "Matrix44.h"
#include "Transformation.h"

namespace geom
{
	// relative space types
	namespace rel
	{
		typedef float Scalar;
		typedef Vector<Scalar, 2> Vector2;
		typedef Vector<Scalar, 3> Vector3;
		typedef Ray<Scalar, 3> Ray3;
		typedef Sphere<Scalar, 3> Sphere3;
		typedef Matrix<Scalar, 3, 3> Matrix33;
		typedef Matrix<Scalar, 4, 4> Matrix44;
		typedef Transformation<Scalar> Transformation;
	}

	// absolute space types
	namespace abs
	{
		typedef double Scalar;
		typedef Vector<Scalar, 2> Vector2;
		typedef Vector<Scalar, 3> Vector3;
		typedef Ray<Scalar, 3> Ray3;
		typedef Sphere<Scalar, 3> Sphere3;
		typedef Matrix<Scalar, 3, 3> Matrix33;
		typedef Matrix<Scalar, 4, 4> Matrix44;
		typedef Transformation<Scalar> Transformation;
	}

	// relative-absolute conversion
	template <typename REL_S = geom::rel::Scalar>
	inline Vector<REL_S, 3> AbsToRel(geom::abs::Vector3 const & abs, geom::abs::Vector3 const & origin)
	{
		return Cast<REL_S>(abs - origin);
	}
	
	template <typename REL_S = geom::rel::Scalar>
	inline Ray<REL_S, 3> AbsToRel(geom::abs::Ray3 const & abs, geom::abs::Vector3 const & origin)
	{
		return Ray<REL_S, 3>(AbsToRel<REL_S>(abs.position, origin), Cast<REL_S>(abs.direction));
	}
	
	template <typename REL_S = geom::rel::Scalar>
	inline Sphere<REL_S, 3> AbsToRel(geom::abs::Sphere3 const & abs, geom::abs::Vector3 const & origin)
	{
		return Sphere<REL_S, 3>(AbsToRel<REL_S>(abs.center, origin), Cast<REL_S>(abs.radius));
	}
	
	template <typename REL_S = geom::rel::Scalar>
	inline geom::abs::Vector3 RelToAbs(Vector<REL_S, 3> const & rel, geom::abs::Vector3 const & origin)
	{
		return Cast<geom::abs::Scalar>(rel) + origin;
	}
	
	template <typename REL_S = geom::rel::Scalar>
	inline geom::abs::Ray3 RelToAbs(Ray<REL_S, 3> const & rel, geom::abs::Vector3 const & origin)
	{
		return geom::abs::Ray3(RelToAbs<REL_S>(rel.position, origin), Cast<geom::abs::Scalar>(rel.direction));
	}
	
	template <typename REL_S = geom::rel::Scalar>
	inline geom::abs::Sphere3 RelToAbs(Sphere<REL_S, 3> const & rel, geom::abs::Vector3 const & origin)
	{
		return geom::abs::Sphere3(RelToAbs<REL_S>(rel.center, origin), static_cast<geom::abs::Scalar>(rel.radius));
	}
}

//
//  axes.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Transformation.h"
#include "geom/Ray.h"
#include "geom/Sphere.h"

namespace axes 
{
	enum Axis
	{
		RIGHT,
		FORWARD,
		UP,
		NUM_AXES
	};
	
	// relative geometry
	typedef ::geom::Vector<float, 3> VectorRel;
	typedef ::geom::Ray<float, 3> RayRel;
	
	// absolute vectors
	typedef ::geom::Vector<double, 3> VectorAbs;
	typedef ::geom::Ray<double, 3> RayAbs;
	typedef ::geom::Sphere<double, 3> SphereAbs;

	// relative-absolute conversion
	template <typename REL_S = float>
	inline geom::Vector<REL_S, 3> AbsToRel(VectorAbs const & abs, VectorAbs const & origin)
	{
		return geom::Cast<REL_S>(abs - origin);
	}
	
	template <typename REL_S = float>
	inline geom::Ray<REL_S, 3> AbsToRel(RayAbs const & abs, VectorAbs const & origin)
	{
		return geom::Ray<REL_S, 3>(AbsToRel<REL_S>(abs.position, origin), geom::Cast<REL_S>(abs.direction));
	}
	
	template <typename REL_S = float>
	inline VectorAbs RelToAbs(geom::Vector<REL_S, 3> const & rel, VectorAbs const & origin)
	{
		return geom::Cast<double>(rel) + origin;
	}
	
	template <typename REL_S = float>
	inline RayAbs RelToAbs(RayRel const & rel, VectorAbs const & origin)
	{
		return RayAbs(RelToAbs(rel.position, origin), geom::Cast<double>(rel.direction));
	}
	
	// Returns the given axis from the given matrix.
	template<typename S> inline geom::Vector<S, 3> GetAxis(geom::Matrix<S, 3, 3> const & rotation, Axis axis)
	{
		return geom::Vector<S, 3>(rotation[0][axis], rotation[1][axis], rotation[2][axis]);
	}
	
	// Converts position/matrix combo to a Ray.
	template<typename S> 
	geom::Ray<S, 3> GetCameraRay(geom::Transformation<S> const & transformation)
	{
		return geom::Ray<S, 3>(transformation.GetTranslation(), GetAxis(transformation.GetRotation(), FORWARD));
	}
	
	// converts forward and up vectors into rotation matrix
	template<typename S>
	geom::Matrix<S, 3, 3> Rotation(geom::Vector<S, 3> const & forward, geom::Vector<S, 3> const & up)
	{
		ASSERT(NearEqual(Length(forward), S(1), S(0.0001)));
		
		geom::Vector<S, 3> right = Normalized(CrossProduct(forward, up));
		geom::Vector<S, 3> matrix_up = CrossProduct(right, forward);
		
		return geom::Matrix<S, 3, 3>(right.x, right.y, right.z,
							   forward.x, forward.y, forward.z,
							   matrix_up.x, matrix_up.y, matrix_up.z);
	}
	
	// converts forward vector into rotation matrix
	template<typename S>
	geom::Matrix<S, 3, 3> Rotation(geom::Vector<S, 3> const & forward)
	{
		ASSERT(NearEqual(Length(forward), S(1), S(0.0001)));

		geom::Vector<S, 3> up = Perpendicular(forward);
		Normalize(up);
		
		geom::Vector<S, 3> right = CrossProduct(forward, up);
		ASSERT(NearEqual(Length(right), S(1), S(0.0001)));
		
		// verify axes are perpendicular to one another
		ASSERT(NearEqual(DotProduct(forward, right), S(0), S(0.0001)));
		ASSERT(NearEqual(DotProduct(right, up), S(0), S(0.0001)));
		ASSERT(NearEqual(DotProduct(up, forward), S(0), S(0.0001)));

		return geom::Matrix<S, 3, 3>(right.x, right.y, right.z,
							   forward.x, forward.y, forward.z,
							   up.x, up.y, up.z);
	}
}

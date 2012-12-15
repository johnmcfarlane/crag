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
	typedef float ScalarRel;
	typedef ::geom::Vector<ScalarRel, 3> VectorRel;
	typedef ::geom::Ray<ScalarRel, 3> RayRel;
	typedef ::geom::Sphere<ScalarRel, 3> SphereRel;
	
	// absolute vectors
	typedef double ScalarAbs;
	typedef ::geom::Vector<ScalarAbs, 3> VectorAbs;
	typedef ::geom::Ray<ScalarAbs, 3> RayAbs;
	typedef ::geom::Sphere<ScalarAbs, 3> SphereAbs;

	// relative-absolute conversion
	template <typename REL_S = ScalarRel>
	inline geom::Vector<REL_S, 3> AbsToRel(VectorAbs const & abs, VectorAbs const & origin)
	{
		return geom::Cast<REL_S>(abs - origin);
	}
	
	template <typename REL_S = ScalarRel>
	inline geom::Ray<REL_S, 3> AbsToRel(RayAbs const & abs, VectorAbs const & origin)
	{
		return geom::Ray<REL_S, 3>(AbsToRel<REL_S>(abs.position, origin), geom::Cast<REL_S>(abs.direction));
	}
	
	template <typename REL_S = ScalarRel>
	inline geom::Sphere<REL_S, 3> AbsToRel(SphereAbs const & abs, VectorAbs const & origin)
	{
		return geom::Sphere<REL_S, 3>(AbsToRel<REL_S>(abs.center, origin), geom::Cast<REL_S>(abs.radius));
	}
	
	template <typename REL_S = ScalarRel>
	inline VectorAbs RelToAbs(geom::Vector<REL_S, 3> const & rel, VectorAbs const & origin)
	{
		return geom::Cast<ScalarAbs>(rel) + origin;
	}
	
	template <typename REL_S = ScalarRel>
	inline RayAbs RelToAbs(geom::Ray<REL_S, 3> const & rel, VectorAbs const & origin)
	{
		return RayAbs(RelToAbs<REL_S>(rel.position, origin), geom::Cast<ScalarAbs>(rel.direction));
	}
	
	template <typename REL_S = ScalarRel>
	inline SphereAbs RelToAbs(geom::Sphere<REL_S, 3> const & rel, VectorAbs const & origin)
	{
		return SphereAbs(RelToAbs<REL_S>(rel.center, origin), static_cast<ScalarAbs>(rel.radius));
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

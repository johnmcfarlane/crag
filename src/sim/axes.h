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


namespace axes 
{
	enum Axis
	{
		RIGHT,
		FORWARD,
		UP,
		NUM_AXES
	};
	
	// Returns the given axis from the given matrix.
	template<typename S> inline geom::Vector<S, 3> const & GetAxis(geom::Matrix<S, 3, 3> const & rotation, Axis axis)
	{
		return reinterpret_cast<geom::Vector<S, 3> const &> (rotation.GetRow(axis));
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

//
//  axes.h
//  crag
//
//  Created by John on 1/24/10.
//  Copyright 2009-2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <geom/Transformation.h>
#include <geom/utils.h>

#include <core/Random.h>

namespace gfx 
{
	using geom::Direction;

	// Converts position/matrix combo to a Ray.
	template<typename S> 
	geom::Ray<S, 3> GetCameraRay(geom::Transformation<S> const & transformation)
	{
		return geom::Ray<S, 3>(transformation.GetTranslation(), GetAxis(transformation.GetRotation(), Direction::forward));
	}

	// the depth value pertinent to an OpenGL depth range
	template <typename S>
	S GetDepth(geom::Transformation<S> const & transformation)
	{
		auto & matrix = transformation.GetMatrix();
		return matrix[2][3];
	}
	
	// converts matrix from world space to OpenGL space
	template<typename S>
	geom::Matrix<S, 4, 4> ToOpenGl(geom::Matrix<S, 4, 4> const & matrix)
	{
		return geom::Matrix<S, 4, 4>(matrix[0], matrix[1], - matrix[2], matrix[3]);
	}

	// converts vector from world space to OpenGL space
	template<typename S>
	geom::Vector<S, 3> ToOpenGl(geom::Vector<S, 3> const & vector)
	{
		return geom::Vector<S, 3>(vector[0], vector[1], - vector[2]);
	}

	// converts vector from world space to OpenGL space
	template<typename S>
	geom::Vector<S, 4> ToOpenGl(geom::Vector<S, 4> const & vector)
	{
		return geom::Vector<S, 4>(vector[0], vector[1], - vector[2], vector[3]);
	}
}


/*
 *  defs.h
 *  crag
 *
 *  Created by John on 6/25/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/Vector3.h"
#include "geom/Vector4.h"
#include "geom/Ray3.h"
#include "geom/Sphere3.h"


namespace form
{
	// Vector Types
	typedef float Scalar;
	
	typedef ::Vector<Scalar, 3>	Vector3;
	typedef ::Vector<Scalar, 4>	Vector4;
	typedef ::Ray<Scalar, 3>	Ray3;
	typedef ::Sphere<Scalar, 3>	Sphere3;
	
	// Conversions between supergalactic (sim) and observer (form) coordinate system.
	template <typename V> V SimToScene(V const & sim, V const & scene_origin)
	{
		return sim - scene_origin;
	}
	template <typename V> V SceneToSim(V const & scene, V const & scene_origin)
	{
		return scene + scene_origin;
	}
}

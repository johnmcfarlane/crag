//
//  defs.h
//  crag
//
//  Created by John on 6/25/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Ray.h"
#include "geom/Sphere.h"

#include "gfx/Color.h"

#include "smp/vector.h"


namespace form
{
	// forward-delcares
	class Node;
	
	// Vector Types
	typedef float Scalar;
	
	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Vector<Scalar, 4> Vector4;
	typedef geom::Ray<Scalar, 3> Ray3;
	typedef geom::Sphere<Scalar, 3> Sphere3;
	typedef gfx::Color4b Color;
	
	// thread-safe node vector
	typedef smp::vector<Node *> SmpNodeVector;
	
	// Conversions between supergalactic (sim) and observer (form) coordinate system.
	inline Vector3 SimToScene(geom::Vector3d const & sim, geom::Vector3d const & scene_origin)
	{
		return sim - scene_origin;
	}
	inline geom::Vector3d SceneToSim(Vector3 const & scene, geom::Vector3d const & scene_origin)
	{
		return geom::Vector3d(scene) + scene_origin;
	}
}

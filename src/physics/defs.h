//
//  defs.h
//  crag
//
//  Created by John on 6/20/10.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <ode/collision.h>
#include <ode/common.h>
#include <ode/contact.h>
#include <ode/mass.h>

namespace geom
{
	template <typename S, int R, int C>
	class Matrix;
	template <typename S>
	class Transformation;
}

////////////////////////////////////////////////////////////////////////////////
// definitions

namespace physics
{
	typedef dReal Scalar;
	typedef dGeomID CollisionHandle;
	typedef dBodyID BodyHandle;
	typedef dJointID JointHandle;
	typedef dContact Contact;
	typedef dContactGeom ContactGeom;
	typedef dMass Mass;
	typedef dTriMeshDataID MeshData;

	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Ray<Scalar, 3> Ray3;
	typedef geom::Sphere<Scalar, 3> Sphere3;
	typedef geom::Matrix<Scalar, 3, 3> Matrix33;
	typedef geom::Matrix<Scalar, 4, 4> Matrix44;
	typedef geom::Transformation<Scalar> Transformation;
	typedef geom::Triangle<Scalar, 3> Triangle3;

	inline void Convert(dVector3 & lhs, Vector3 const & rhs)
	{
		memcpy(lhs, & rhs, sizeof(Vector3));
	}

	inline Vector3 const & Convert(dVector3 const & v) 
	{ 
		return reinterpret_cast<Vector3 const &>(v);
	}
}

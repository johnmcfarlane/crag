//
//  defs.h
//  crag
//
//  Created by John on 6/20/10.
//  Copyright 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

////////////////////////////////////////////////////////////////////////////////
// forward-declarations

#if defined(USE_BULLET)
class btContactConstraint;
#endif

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
#if defined(USE_ODE)
	typedef dReal Scalar;
	typedef dGeomID CollisionHandle;
	typedef dBodyID BodyHandle;
	typedef dJointID JointHandle;
	typedef dContact Contact;
#endif

#if defined(USE_BULLET)
	typedef btScalar Scalar;
	typedef btCollisionObject * CollisionHandle;
	typedef btRigidBody * BodyHandle;
	typedef btTypedConstraint * JointHandle;
	typedef btContactConstraint Contact;
#endif

	typedef geom::Vector<Scalar, 3> Vector3;
	typedef geom::Ray<Scalar, 3> Ray3;
	typedef geom::Matrix<Scalar, 3, 3> Matrix33;
	typedef geom::Matrix<Scalar, 4, 4> Matrix44;
	typedef geom::Transformation<Scalar> Transformation;
}

/*
 *  Sphere3.h
 *  Crag
 *
 *  Created by John on 10/31/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "geom/NSphere.h"
#include "geom/Vector3.h"
#include "core/floatOps.h"

template<typename S> class Sphere3 : public Sphere<Vector<S, 3>, S>
{
public:
	typedef Vector<S, 3> Vector;
	typedef Sphere<Vector, S> Base;
	
	Sphere3() { }
	
	Sphere3(Vector const & c, S r) : Sphere<Vector, S>(c, r) { }
	
	// templated copy constructor - can take a Sphere3 of a different type
	template<typename I> Sphere3(Sphere3<I> const & rhs) 
	: Base(rhs.center, rhs.radius) { }
	
	S Area() const {
		return static_cast<S>(PI * 4.) * Square(Base::GetRadius());
	}
	
	S Volume() const {
		return static_cast<S>(PI * 4. / 3.) * Cube(Base::GetRadius());
	}
};

typedef Sphere3<float> Sphere3f;

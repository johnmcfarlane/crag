/*
 *  Circle2.h
 *  Crag
 *
 *  Created by John on 10/31/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 */

#include "Sphere.h"
#include "geom/Vector2.h"

template<typename S> class Circle2 : public Sphere<Vector2<S>, S>
{
public:
	typedef Vector2<S> Vector;
	
	Circle2(Vector const & c, S r) : Sphere<Vector, S>(c, r) { }

	S Circumference() const {
		return static_cast<S>(PI * 2.) * GetRadius();
	}

	S Area() const {
		return static_cast<S>(PI) * Square(GetRadius());
	}
};

class Circle2f : public Circle2<float>
{
public:
	Circle2f(Vector2f const & c, float r) : Circle2<float>(c, r) { }
};


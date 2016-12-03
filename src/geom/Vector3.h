//
//  Vector3.h
//  crag
//
//  Created by john on 5/14/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Vector.h"


namespace geom
{
	//////////////////////////////////////////////////////////////////
	// 3-dimensional partial specialization of Vector

	template<typename S> class Vector<S, 3>
	: public glm::tvec3<S>
	{
//	public:
//		typedef S Scalar;
//
//		Vector()
//#if defined(CRAG_DEBUG)
//		: x(std::numeric_limits<S>::signaling_NaN())
//		, y(std::numeric_limits<S>::signaling_NaN())
//		, z(std::numeric_limits<S>::signaling_NaN())
//#endif
//		{
//		}
//
//		template <typename RHS_S>
//		constexpr explicit Vector(Vector<RHS_S, 3> const & rhs)
//				: Vector(rhs.x, rhs.y, rhs.z)
//		{
//		}
//
//		template <typename RHS_S>
//		constexpr explicit Vector(RHS_S rhs_x, RHS_S rhs_y, RHS_S rhs_z)
//			: x(static_cast<S>(rhs_x))
//			, y(static_cast<S>(rhs_y))
//			, z(static_cast<S>(rhs_z))
//		{
//		}
//
//		template <typename LHS_S>
//		constexpr explicit operator Vector<LHS_S, 3>() const
//		{
//			return Vector<LHS_S, 3>(x, y, z);
//		}
//
//		static constexpr std::size_t size()
//		{
//			return 3;
//		}
//
//		// Returns vector as a C-style array. Very unsafe.
//		S const & operator[](int index) const
//		{
//			ASSERT(index >= 0 && index < 3);
//			return data() [index];
//		}
//
//		S & operator[](int index)
//		{
//			ASSERT(index >= 0 && index < 3);
//			return data() [index];
//		}
//
//		S * data()
//		{
//			return reinterpret_cast<S *>(this);
//		}
//
//		S const * data() const
//		{
//			return reinterpret_cast<S const *>(this);
//		}
//
//		static Vector const & Zero()
//		{
//			static constexpr auto zero_vector = Vector(S(0), S(0), S(0));
//			return zero_vector;
//		}
//
//		static Vector Max()
//		{
//			S max_value = std::numeric_limits<S>::max();
//			return Vector(max_value, max_value, max_value);
//		}
//
//#if defined(CRAG_VERIFY_ENABLED)
//		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Vector, v)
//			CRAG_VERIFY(v.x);
//			CRAG_VERIFY(v.y);
//			CRAG_VERIFY(v.z);
//		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
//#endif
//
//		S x, y, z;
	};

//	template <typename S>
//	Vector<S, 3> constexpr MakeVector(S x, S y, S z)
//	{
//		return Vector<S, 3>(x, y, z);
//	}
//
//	template<typename S> bool operator == (Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
//	{
//		return
//			lhs.x == rhs.x &&
//			lhs.y == rhs.y &&
//			lhs.z == rhs.z;
//	}
//
//	template<typename S> inline Vector<S, 3> & operator += (Vector<S, 3> & lhs, Vector<S, 3> const & rhs)
//	{
//		lhs.x += rhs.x;
//		lhs.y += rhs.y;
//		lhs.z += rhs.z;
//		return lhs;
//	}
//
//	template<typename S> inline Vector<S, 3> & operator -= (Vector<S, 3> & lhs, Vector<S, 3> const & rhs)
//	{
//		lhs.x -= rhs.x;
//		lhs.y -= rhs.y;
//		lhs.z -= rhs.z;
//		return lhs;
//	}
//
//	template<typename S> inline Vector<S, 3> & operator *= (Vector<S, 3> & lhs, S rhs)
//	{
//		lhs.x *= rhs;
//		lhs.y *= rhs;
//		lhs.z *= rhs;
//		return lhs;
//	}
//
//	template<typename S> inline Vector<S, 3> & operator *= (Vector<S, 3> & lhs, Vector<S, 3> const & rhs)
//	{
//		lhs.x *= rhs.x;
//		lhs.y *= rhs.y;
//		lhs.z *= rhs.z;
//		return lhs;
//	}
//
//	template<typename S> inline Vector<S, 3> & operator /= (Vector<S, 3> & lhs, Vector<S, 3> const & rhs)
//	{
//		lhs.x /= rhs.x;
//		lhs.y /= rhs.y;
//		lhs.z /= rhs.z;
//		return lhs;
//	}
//
//	template<typename S> inline Vector<S, 3> operator - (Vector<S, 3> const & rhs)
//	{
//		return Vector<S, 3>(- rhs.x, - rhs.y, - rhs.z);
//	}
//
//	template<typename S> S DotProduct(Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
//	{
//		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
//	}
//
//	template<typename S> Vector<S, 3> CrossProduct(Vector<S, 3> const & lhs, Vector<S, 3> const & rhs)
//	{
//		return Vector<S, 3>(
//			lhs.y * rhs.z - lhs.z * rhs.y,
//			lhs.z * rhs.x - lhs.x * rhs.z,
//			lhs.x * rhs.y - lhs.y * rhs.x);
//	}
//
//	// returns v with all axes set to absolute values
//	template <typename S>
//	Vector<S, 3> Abs(Vector<S, 3> const & v)
//	{
//		return Vector<S, 3>(std::abs(v.x), std::abs(v.y), std::abs(v.z));
//	}
//
//	// selects a vector from the plane to which v is perpendicular
//	template<typename S> Vector<S, 3> Perpendicular(Vector<S, 3> const & v)
//	{
//		typedef Vector<S, 3> Vector;
//
//		Vector a = Abs(v);
//		unsigned uyx = Signbit(a.x - a.y);
//		unsigned uzx = Signbit(a.x - a.z);
//		unsigned uzy = Signbit(a.y - a.z);
//
//		unsigned xm = uyx & uzx;
//		unsigned ym = (1^xm) & uzy;
//		unsigned zm = 1^(xm | ym);
//
//		Vector p = CrossProduct(v, Vector(S(xm), S(ym), S(zm)));
//		return p;
//	}
//
//	template<typename S> ::std::ostream & operator << (::std::ostream & out, Vector<S, 3> const & v)
//	{
//		return out << v.x << ',' << v.y << ',' << v.z;
//	}
//
//	template<typename S> ::std::istream & operator >> (::std::istream & in, Vector<S, 3> & v)
//	{
//		return in >> v.x >> ',' >> v.y >> ',' >> v.z;
//	}

	//////////////////////////////////////////////////////////////////
	// Vector<T, 3> aliases

	typedef Vector <int, 3> Vector3i;
	typedef Vector <float, 3> Vector3f;
	typedef Vector <double, 3> Vector3d;
}

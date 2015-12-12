//
//  Vector4.h
//  crag
//
//  Created by john on 5/14/09.
//  Copyright 2009 - 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Vector.h"


namespace geom
{
	//////////////////////////////////////////////////////////////////
	// 4-dimensional partical specialization of Vector

	template<typename S> class Vector<S, 4>
	{
	public:
		Vector() noexcept
#if defined(CRAG_DEBUG)
		: Vector(
			std::numeric_limits<S>::signaling_NaN(),
			std::numeric_limits<S>::signaling_NaN(),
			std::numeric_limits<S>::signaling_NaN(),
			std::numeric_limits<S>::signaling_NaN())
#endif
		{
		}

		template <typename RHS_S>
		constexpr explicit Vector(Vector<RHS_S, 4> && rhs) noexcept
			: Vector(rhs.x, rhs.y, rhs.z, rhs.w) {}

		template <typename RHS_S>
		constexpr explicit Vector(RHS_S const rhs[4]) noexcept
			: Vector(rhs[0], rhs[1], rhs[2], rhs[3])
		{
		}

		template <typename RHS_S>
		constexpr explicit Vector(RHS_S rhs_x, RHS_S rhs_y, RHS_S rhs_z, RHS_S rhs_w) noexcept
		: x(static_cast<S>(rhs_x))
		, y(static_cast<S>(rhs_y))
		, z(static_cast<S>(rhs_z))
		, w(static_cast<S>(rhs_w))
		{
		}
		
		// Returns vector as a C-style array. Very unsafe. 
		S const & operator[](int index) const 
		{
			ASSERT(index >= 0);
			ASSERT(index < 4);
			return data() [index];
		} 
		
		S & operator[](int index) 
		{
			ASSERT(index >= 0);
			ASSERT(index < 4);
			return data() [index];
		} 
		
		S * data()
		{
			return reinterpret_cast<S *>(this);
		}
		
		S const * data() const
		{
			return reinterpret_cast<S const *>(this);
		}
		
		static Vector Zero() 
		{
			return Vector(0, 0, 0, 0); 
		}
		
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Vector, v)
			CRAG_VERIFY(v.x);
			CRAG_VERIFY(v.y);
			CRAG_VERIFY(v.z);
			CRAG_VERIFY(v.w);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif
		
		S x, y, z, w;
	};


	template<typename S> bool operator == (Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
	{
		return
		lhs.x == rhs.x && 
		lhs.y == rhs.y && 
		lhs.z == rhs.z && 
		lhs.w == rhs.w;
	}

	template<typename S> Vector<S, 4> & operator += (Vector<S, 4> & lhs, Vector<S, 4> const & rhs)
	{
		lhs.x += rhs.x;
		lhs.y += rhs.y;
		lhs.z += rhs.z;
		lhs.w += rhs.w;
		return lhs;
	}

	template<typename S> Vector<S, 4> & operator -= (Vector<S, 4> & lhs, Vector<S, 4> const & rhs)
	{
		lhs.x -= rhs.x;
		lhs.y -= rhs.y;
		lhs.z -= rhs.z;
		lhs.w -= rhs.w;
		return lhs;
	}

	template<typename S> Vector<S, 4> & operator *= (Vector<S, 4> & lhs, S rhs)
	{
		lhs.x *= rhs;
		lhs.y *= rhs;
		lhs.z *= rhs;
		lhs.w *= rhs;
		return lhs;
	}

	template<typename S> Vector<S, 4> operator - (Vector<S, 4> const & rhs)
	{
		return Vector<S, 4>(- rhs.x, - rhs.y, - rhs.z, - rhs.w);
	}

	template<typename S> S DotProduct(Vector<S, 4> const & lhs, Vector<S, 4> const & rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	template<typename S> ::std::ostream & operator << (::std::ostream & out, Vector<S, 4> const & v)
	{
		return out << v.x << ',' << v.y << ',' << v.z << ',' << v.w;
	}

	template<typename S> ::std::istream & operator >> (::std::istream & in, Vector<S, 4> const & v)
	{
		return in >> v.x >> ',' >> v.y >> ',' >> v.z >> ',' >> v.w;
	}
	
	//////////////////////////////////////////////////////////////////
	// Vector<T, 4> aliases
	
	typedef Vector<int, 4> Vector4i;
	typedef Vector<float, 4> Vector4f;
	typedef Vector<double, 4> Vector4d;
}

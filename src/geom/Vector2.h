//
//  Vector2.h
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
	// Vector2 - 2-dimensional vector

	// 2-dimensional partical specialization of Vector
	template<typename S> class Vector<S, 2>
	{
	public:
		typedef S Scalar;
		
		Vector() noexcept
#if defined(CRAG_DEBUG)
		: x(std::numeric_limits<S>::signaling_NaN())
		, y(std::numeric_limits<S>::signaling_NaN())
#endif
		{ 
		}

		template <typename RHS_S>
		constexpr explicit Vector(Vector<RHS_S, 2> const & rhs) noexcept
		: Vector(rhs.x, rhs.y) {
		}

		template <typename RHS_S>
		constexpr explicit Vector(RHS_S const & rhs_x, RHS_S const & rhs_y) noexcept
		: x(static_cast<S>(rhs_x))
		, y(static_cast<S>(rhs_y)) {
		}

		static constexpr std::size_t size() noexcept {
			return 3;
		}
		
		// Returns vector as a C-style array. Very unsafe. 
		constexpr S const * data() const noexcept {
			return reinterpret_cast<S const *>(this);
		}	
		S * data() noexcept {
			return reinterpret_cast<S *>(this);
		}
		
		S const & operator [] (int index) const
		{
			ASSERT(index >= 0 && index < 2);
			return data() [index];
		}
		S & operator [] (int index)
		{
			ASSERT(index >= 0 && index < 2);
			return data() [index];
		}
		
		static Vector Zero() 
		{
			return Vector(0,0); 
		}
		
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Vector, v)
			CRAG_VERIFY(v.x);
			CRAG_VERIFY(v.y);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

		S x, y;
	};

	template <typename S>
	Vector<S, 2> MakeVector(S x, S y)
	{
		return Vector<S, 2>(x, y);
	}

	template<typename S> bool operator == (Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		return
			lhs.x == rhs.x && 
			lhs.y == rhs.y;
	}

	template<typename S> Vector<S, 2> & operator += (Vector<S, 2> & lhs, Vector<S, 2> const & rhs)
	{
		lhs.x += rhs.x;
		lhs.y += rhs.y;
		return lhs;
	}

	template<typename S> Vector<S, 2> & operator -= (Vector<S, 2> & lhs, Vector<S, 2> const & rhs)
	{
		lhs.x -= rhs.x;
		lhs.y -= rhs.y;
		return lhs;
	}

	template<typename S> Vector<S, 2> & operator *= (Vector<S, 2> & lhs, Vector<S, 2> const & rhs)
	{
		lhs.x *= rhs.x;
		lhs.y *= rhs.y;
		return lhs;
	}

	template<typename S> Vector<S, 2> & operator *= (Vector<S, 2> & lhs, S rhs)
	{
		lhs.x *= rhs;
		lhs.y *= rhs;
		return lhs;
	}

	template<typename S> Vector<S, 2> & operator /= (Vector<S, 2> & lhs, Vector<S, 2> const & rhs)
	{
		lhs.x /= rhs.x;
		lhs.y /= rhs.y;
		return lhs;
	}

	template<typename S> S DotProduct(Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	
	template<typename S> ::std::ostream & operator << (::std::ostream & out, Vector<S, 2> const & v)
	{
		return out << v.x << ',' << v.y;
	}

	template<typename S> ::std::istream & operator >> (::std::istream & in, Vector<S, 2> const & v)
	{
		return in >> v.x >> ',' >> v.y;
	}

	//////////////////////////////////////////////////////////////////
	// Vector<T, 2> aliases
	
	typedef Vector<int, 2> Vector2i;
	typedef Vector<float, 2> Vector2f;
	typedef Vector<double, 2> Vector2d;
}

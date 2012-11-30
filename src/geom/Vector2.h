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
		Vector() 
#if ! defined(NDEBUG)
		: x(std::numeric_limits<S>::signaling_NaN())
		, y(std::numeric_limits<S>::signaling_NaN())
#endif
		{ 
		}
		
		template<typename RHS_S> 
		Vector(Vector<RHS_S, 2> const & rhs) 
		: Vector(rhs.x, rhs.y)
		{ 
		}
		
		template<typename RHS_S> 
		Vector(RHS_S rhs_x, RHS_S rhs_y) 
		: x(static_cast<S>(rhs_x))
		, y(static_cast<S>(rhs_y))
		{ 
		}

		// Returns vector as a C-style array. Very unsafe. 
		S const * GetAxes() const
		{
			return reinterpret_cast<S const *>(this);
		}	
		S * GetAxes()
		{
			return reinterpret_cast<S *>(this);
		}
		
		S const & operator [] (int index) const
		{
			ASSERT(index >= 0 && index < 2);
			return GetAxes() [index];
		}
		S & operator [] (int index)
		{
			ASSERT(index >= 0 && index < 2);
			return GetAxes() [index];
		}
		
		static Vector Zero() 
		{
			return Vector(0,0); 
		}
		
#if defined(VERIFY)
		void Verify() const
		{
			VerifyTrue(x == x);
			VerifyTrue(y == y);
		}
#endif
		
		S x, y;
	};


	template<typename S> bool operator == (Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		return
			lhs.x == rhs.x && 
			lhs.y == rhs.y;
	}

	template<typename S> bool operator != (Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		return ! (lhs == rhs);
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

	template<typename LHS_S, typename RHS_S> Vector<LHS_S, 2> & operator *= (Vector<LHS_S, 2> & lhs, RHS_S rhs)
	{
		lhs.x *= rhs;
		lhs.y *= rhs;
		return lhs;
	}

	template<typename S> Vector<S, 2> & operator /= (Vector<S, 2> & lhs, S rhs)
	{
		lhs.x /= rhs;
		lhs.y /= rhs;
		return lhs;
	}

	template<typename S> Vector<S, 2> operator - (Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		Vector<S, 2> result = lhs;
		return result -= rhs;
	}

	template<typename S> Vector<S, 2> operator + (Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		Vector<S, 2> result = lhs;
		return result += rhs;
	}

	template<typename S> Vector<S, 2> operator * (Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		Vector<S, 2> result = lhs;
		return result *= rhs;
	}

	template<typename S> Vector<S, 2> operator * (Vector<S, 2> const & lhs, S rhs)
	{
		Vector<S, 2> result = lhs;
		return result *= rhs;
	}

	template<typename S> Vector<S, 2> operator / (Vector<S, 2> const & lhs, S rhs)
	{
		Vector<S, 2> result = lhs;
		return result /= rhs;
	}

	template<typename S> S LengthSq(Vector<S, 2> const & v)
	{
		return v.x * v.x + v.y * v.y;
	}

	template<typename S> S DotProduct(Vector<S, 2> const & lhs, Vector<S, 2> const & rhs)
	{
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}
	
	
	//////////////////////////////////////////////////////////////////
	// specializations of Vector2
	
	typedef Vector<float, 2> Vector2f;
	typedef Vector<int, 2> Vector2i;
}

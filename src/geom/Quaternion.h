//
//  Quaternion.h
//  crag
//
//  Created by john on 5/14/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#if FINISHED_AND_WORKING

#include "Matrix33.h"
#include "Vector3.h"

namespace geom
{
	////////////////////////////////////////////////////////////////////////////////
	// Quaternion - efficient rotation

	template<typename S> 
	class Quaternion
	{
	public:
		typedef S Scalar;
		typedef ::geom::Vector<S, 3> Vector;
		typedef ::geom::Matrix<S, 3, 3> Matrix;
		
		Quaternion() 
#if defined(CRAG_DEBUG)
		: w(std::numeric_limits<S>::signaling_NaN())
#endif
		{ 
		}

		Quaternion(Quaternion const &) = default;
		
		Quaternion(Vector const & rhs_v, Scalar rhs_w) 
		: v(rhs_v)
		, w(rhs_w) 
		{ 
			CRAG_VERIFY(* this);
		}
		
		explicit Quaternion(Matrix const & _rhs)
		{
			auto rhs = /*Transposition*/(_rhs);
			Scalar tr,s;
			tr = rhs[0][0] + rhs[1][1] + rhs[2][2];
			if (tr >= 0) 
			{
				s = std::sqrt (tr + 1);
				w = 0.5 * s;
				s = 0.5 / s;
				v.x = (rhs[2][1] - rhs[1][2]) * s;
				v.y = (rhs[0][2] - rhs[2][0]) * s;
				v.z = (rhs[1][0] - rhs[0][1]) * s;
			}
			else 
			{
				// find the largest diagonal element and jump to the appropriate case
				if (rhs[1][1] > rhs[0][0])
				{
					if (rhs[2][2] < rhs[1][1]) 
					{
						s = std::sqrt((rhs[1][1] - (rhs[2][2] + rhs[0][0])) + 1);
						v.y = 0.5 * s;
						s = 0.5 / s;
						v.z = (rhs[1][2] + rhs[2][1]) * s;
						v.x = (rhs[0][1] + rhs[1][0]) * s;
						w = (rhs[0][2] - rhs[2][0]) * s;
						return;
					}
				}
				else
				{
					if (rhs[2][2] < rhs[0][0]) 
					{
						s = std::sqrt((rhs[0][0] - (rhs[1][1] + rhs[2][2])) + 1);
						v.x = 0.5 * s;
						s = 0.5 / s;
						v.y = (rhs[0][1] + rhs[1][0]) * s;
						v.z = (rhs[2][0] + rhs[0][2]) * s;
						w = (rhs[2][1] - rhs[1][2]) * s;
						return;
					}
				}

				s = std::sqrt((rhs[2][2] - (rhs[0][0] + rhs[1][1])) + 1);
				v.z = 0.5 * s;
				s = 0.5 / s;
				v.x = (rhs[2][0] + rhs[0][2]) * s;
				v.y = (rhs[1][2] + rhs[2][1]) * s;
				w = (rhs[1][0] - rhs[0][1]) * s;
			}
		}

		static constexpr std::size_t Size()
		{
			return 4;
		}
		
		// Returns vector as a C-style array. Very unsafe. 
		S const & operator[](int index) const 
		{
			ASSERT(index >= 0 && index < 4);
			return GetAxes() [index];
		} 
		
		S & operator[](int index) 
		{
			ASSERT(index >= 0 && index < 4);
			return GetAxes() [index];
		} 

		S * GetAxes()
		{
			return reinterpret_cast<S *>(this);
		}

		S const * GetAxes() const
		{
			return reinterpret_cast<S const *>(this);
		}

		static Quaternion Identity() 
		{
			return Quaternion(Vector(0, 0, 0), 1); 
		}
		
		static Quaternion SetAxisAngle(Vector const & axis, Scalar angle)
		{
			ASSERT(NearEqual(Magnitude(axis), 1, 0.001));

			angle *= 0.5;
			auto s = std::sin(angle);
			return Quaternion(axis * s, std::cos (angle));
		}
		
		Vector GetAxis() const
		{
			Scalar ss = Scalar(1) - w * w;
			if (ss < 0.0001)
			{
				ASSERT(false);
				return Vector(1.0, 0.0, 0.0);  // Arbitrary
			}
			
			Scalar s = Scalar(1) / std::sqrt(ss);
			return v * s;
		}
  
  #if defined(CRAG_VERIFY_ENABLED)
		void Verify() const
		{
			CRAG_VERIFY(v);
			CRAG_VERIFY_TRUE(w == w);
		}
#endif
		
		Vector v;
		Scalar w;
	};

	// casts between quaternion matrices of different scalar types
	template <typename LHS_S, typename RHS_S>
	typename std::enable_if<! std::is_same<LHS_S, RHS_S>::value, Quaternion<LHS_S>>::type Cast(Quaternion<RHS_S> const & rhs)
	{
		return Quaternion<LHS_S>(
			Cast<LHS_S>(rhs.v),
			static_cast<LHS_S>(rhs.w));
	}

	template <typename S> 
	bool operator == (Quaternion<S> const & lhs, Quaternion<S> const & rhs)
	{
		return
			lhs.v == rhs.v && 
			lhs.w == rhs.w;
	}

	template <typename S> 
	bool operator != (Quaternion<S> const & lhs, Quaternion<S> const & rhs)
	{
		return ! (lhs == rhs);
	}

	template <typename S> 
	Quaternion<S> operator * (Quaternion<S> const & lhs, Quaternion<S> const & rhs)
	{
		return Quaternion<S>(
			Vector<S, 3>(
				lhs.w * rhs.v.x + lhs.v.x * rhs.w + lhs.v.y * rhs.v.z - lhs.v.z * rhs.v.y,
				lhs.w * rhs.v.y + lhs.v.y * rhs.w + lhs.v.z * rhs.v.x - lhs.v.x * rhs.v.z,
				lhs.w * rhs.v.z + lhs.v.z * rhs.w + lhs.v.x * rhs.v.y - lhs.v.y * rhs.v.x),
			lhs.w * rhs.w - lhs.v.x * rhs.v.x - lhs.v.y * rhs.v.y - lhs.v.z * rhs.v.z);
	}

	template <typename S> 
	Quaternion<S> operator *= (Quaternion<S> & lhs, Quaternion<S> const & rhs)
	{
		return lhs = lhs * rhs;
	}

	template <typename S> 
	Quaternion<S> operator * (Quaternion<S> const & lhs, Vector<S, 3> const & rhs)
	{
		return btQuaternion( 
			lhs.w * rhs.x + lhs.v.y * rhs.z - lhs.v.z * rhs.y,
			lhs.w * rhs.y + lhs.v.z * rhs.x - lhs.v.x * rhs.z,
			lhs.w * rhs.z + lhs.v.x * rhs.y - lhs.v.y * rhs.x,
			-lhs.v.x * rhs.x - lhs.v.y * rhs.y - lhs.v.z * rhs.z);
	}

	template <typename S> 
	Quaternion<S> operator *= (Quaternion<S> & lhs, Vector<S, 3> const & rhs)
	{
		lhs = lhs * rhs;
	}
	
	template <typename S>
	Quaternion<S> Inverse(Quaternion<S> const & q)
	{
		return Quaternion<S>(- q.v, q.w);
	}
	
	template<typename S> std::ostream & operator << (std::ostream & out, Quaternion<S> const & q)
	{
		return out << q.v.x << ',' << q.v.y << ',' << q.v.z << ',' << q.w;
	}

	template<typename S> std::istream & operator >> (std::istream & in, Quaternion<S> const & q)
	{
		return in >> q.v.x >> ',' >> q.v.y >> ',' >> q.v.z >> ',' >> q.w;
	}
	
	////////////////////////////////////////////////////////////////////////////////
	// 3x3 Matrix member definitions
	
	template <typename Scalar>
	Matrix<Scalar, 3, 3>::Matrix(Quaternion<Scalar> const & rhs)
	{
		Scalar qq1 = 2 * rhs.v.x * rhs.v.x;
		Scalar qq2 = 2 * rhs.v.y * rhs.v.y;
		Scalar qq3 = 2 * rhs.v.z * rhs.v.z;
		
		rows[0] = { 1 - qq2 - qq3, 2 * (rhs.v.x * rhs.v.y - rhs.w * rhs.v.z), 2 * (rhs.v.x * rhs.v.z + rhs.w * rhs.v.y) };
		rows[1] = { 2 * (rhs.v.x * rhs.v.y + rhs.w * rhs.v.z), 1 - qq1 - qq3, 2 * (rhs.v.y * rhs.v.z - rhs.w * rhs.v.x) };
		rows[2] = { 2 * (rhs.v.x * rhs.v.z - rhs.w * rhs.v.y), 2 * (rhs.v.y * rhs.v.z + rhs.w * rhs.v.x), 1 - qq1 - qq2 };
		//*this = Transposition(* this);
	}
}

#endif

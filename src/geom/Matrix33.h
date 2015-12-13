//
//  Matrix33.h
//  crag
//
//  Created by John McFarlane on 9/22/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "geom/Matrix.h"


namespace geom
{
	// 3x3 matrix
	template<typename S> 
	class Matrix<S, 3, 3>
	{
	public:
		// types
		typedef Vector<S, 3> Row;
		typedef Vector<S, 3> Column;
		class _Row : public Vector<S, 3>
		{
			// I would assume that __attribute__ ((aligned (sizeof(S) * 4)))
			// would be enough to ensure the padding in this Matrix class.
			// Alas that doesn't seem the be the case - at least in clang.
			S _padding;
			typedef Vector<S, 3> Row;
		public:
			_Row() { }
			_Row(S a, S b, S c) : Row(a, b, c) { }
			_Row(Row const & rhs) : Row(rhs) { }
		};

		// functions
		Matrix()
		{
			static_assert(sizeof(Matrix) == sizeof(S) * 12, "Matrix<S, 3, 3> is improperly aligned.");
#if defined(CRAG_DEBUG)
			Fill(std::numeric_limits<S>::signaling_NaN());
#endif
		}

		template <typename RHS_S>
		constexpr explicit Matrix(Matrix<RHS_S, 3, 3> const & rhs) noexcept :
			Matrix(rhs[0], rhs[1], rhs[2]) {
		}

		template <typename RHS_S>
		constexpr explicit Matrix(
			typename ::geom::Vector<RHS_S, 3> const & row0,
			typename ::geom::Vector<RHS_S, 3> const & row1,
			typename ::geom::Vector<RHS_S, 3> const & row2) noexcept :
			rows {
				static_cast<Row>(row0),
				static_cast<Row>(row1),
				static_cast<Row>(row2)
			} {
		}

		template <typename RHS_S>
		Matrix(RHS_S m00, RHS_S m01, RHS_S m02,
			   RHS_S m10, RHS_S m11, RHS_S m12,
			   RHS_S m20, RHS_S m21, RHS_S m22) 
		{
			rows[0][0] = m00;
			rows[0][1] = m01; 
			rows[0][2] = m02; 
			rows[1][0] = m10; 
			rows[1][1] = m11; 
			rows[1][2] = m12; 
			rows[2][0] = m20; 
			rows[2][1] = m21; 
			rows[2][2] = m22; 
		}
		
		friend inline bool operator == (Matrix const & lhs, Matrix const & rhs)
		{
			return lhs.rows[0] == rhs.rows[0]
			&& lhs.rows[1] == rhs.rows[1]
			&& lhs.rows[2] == rhs.rows[2];
		}
		
		// Fast accessors - but behavior is different depending on whether matrix is row or column-major
		Row & operator [] (int i) 
		{
			assert(i >= 0 && i < 3);
			return rows[i];
		}
		
		Row const & operator [] (int i) const
		{
			assert(i >= 0 && i < 3);
			return rows[i];
		}
		
		// returns a raw array of numbers;
		// WARNING: contains gaps!
		S * GetArray() 
		{
			return & rows[0][0];
		}
		S const * GetArray() const 
		{
			return & rows[0][0];
		}

		// fast because matrix is row major
		Row const & GetRow(int i) const 
		{ 
			assert(i >= 0 && i < 3);
			return rows[i]; 
		}
		
		// slow because matrix is row major and return value needs to be composed and copied
		Column GetColumn(int i) const 
		{ 
			assert(i >= 0 && i < 3);
			return Column(rows[0][i], rows[1][i], rows[2][i]); 
		}
		
		void SetColumn(int i, Column const & column)
		{ 
			assert(i >= 0 && i < 3);
			rows[0][i] = column[0];
			rows[1][i] = column[1];
			rows[2][i] = column[2];
		}
		
		S Determinant() const
		{
			return (rows[0][0]*rows[1][1]*rows[2][2] 
					+ rows[0][1]*rows[1][2]*rows[2][0] 
					+ rows[0][2]*rows[1][0]*rows[2][1])
				-  (rows[2][0]*rows[1][1]*rows[0][2] 
					+ rows[2][1]*rows[1][2]*rows[0][0] 
					+ rows[2][2]*rows[1][0]*rows[0][1]);
		}
		
		void Fill(S value)
		{
			for (int r = 0; r != 3; ++ r)
			{
				for (int c = 0; c != 3; ++ c)
				{
					rows[r][c] = value;
				}
			}
		}
		
		static Matrix Identity() 
		{
			return Matrix(
				Row(Scalar(1), Scalar(0), Scalar(0)),
				Row(Scalar(0), Scalar(1), Scalar(0)),
				Row(Scalar(0), Scalar(0), Scalar(1)));
		}
		
		static Matrix Zero() 
		{
			return Matrix(Row::Zero(), 
						  Row::Zero(), 
						  Row::Zero());
		}
		
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Matrix, self)
			CRAG_VERIFY(self.rows[0]);
			CRAG_VERIFY(self.rows[1]);
			CRAG_VERIFY(self.rows[2]);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

	private:
		// variables
		_Row rows [3];
	};

	template <typename S>
	Matrix<S, 3, 3> Inverse(Matrix<S, 3, 3> const & matrix)
	{
		Matrix<S, 3, 3> inverse;
		
		S det = matrix.Determinant();
		if(::Abs(det) < std::numeric_limits<S>::min()) 
		{
			return matrix;		// The matrix is not invertible! Singular case!
		}
		
		S inv_det = ::Inverse(det);
		
		inverse[0][0] = +(matrix[1][1] * matrix[2][2] - matrix[2][1] * matrix[1][2]) * inv_det;
		inverse[1][0] = -(matrix[1][0] * matrix[2][2] - matrix[2][0] * matrix[1][2]) * inv_det;
		inverse[2][0] = +(matrix[1][0] * matrix[2][1] - matrix[2][0] * matrix[1][1]) * inv_det;
		inverse[0][1] = -(matrix[0][1] * matrix[2][2] - matrix[2][1] * matrix[0][2]) * inv_det;
		inverse[1][1] = +(matrix[0][0] * matrix[2][2] - matrix[2][0] * matrix[0][2]) * inv_det;
		inverse[2][1] = -(matrix[0][0] * matrix[2][1] - matrix[2][0] * matrix[0][1]) * inv_det;
		inverse[0][2] = +(matrix[0][1] * matrix[1][2] - matrix[1][1] * matrix[0][2]) * inv_det;
		inverse[1][2] = -(matrix[0][0] * matrix[1][2] - matrix[1][0] * matrix[0][2]) * inv_det;
		inverse[2][2] = +(matrix[0][0] * matrix[1][1] - matrix[1][0] * matrix[0][1]) * inv_det;
		
		return inverse;
	}
	
	//////////////////////////////////////////////////////////////////
	// Matrix<T, 3, 3> aliases
	
	typedef Matrix<float, 3, 3> Matrix33f;
	typedef Matrix<double, 3, 3> Matrix33d;
}

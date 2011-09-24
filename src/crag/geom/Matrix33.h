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


// 3x3 matrix

// Column-major in line with OpenGL (and nothing else it would seem).
template<typename S> 
class Matrix<S, 3, 3>
{
public:
	// types
	typedef Vector<S, 3> Row;
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
	}
	
	Matrix(Row const & row0,
		   Row const & row1,
		   Row const & row2)
	{
		rows[0] = row0;
		rows[1] = row1;
		rows[2] = row2;
	}
	
	Matrix(S m00, S m01, S m02, 
		   S m10, S m11, S m12, 
		   S m20, S m21, S m22) 
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
	Row GetColumn(int i) const 
	{ 
		assert(i >= 0 && i < 3);
		return Row(rows[0][i], rows[1][i], rows[2][i]); 
	}
	
	static Matrix Identity() 
	{
		return Matrix(Row(1, 0, 0),
					  Row(0, 1, 0),
					  Row(0, 0, 1)); 
	}
	
	static Matrix Zero() 
	{
		return Matrix(Row::Zero(), 
					  Row::Zero(), 
					  Row::Zero());
	}
	
private:
	// variables
	_Row rows [3];
};


// single-precision floating-point specialization
typedef Matrix<float, 3, 3> Matrix33f;

// double-precision floating-point specialization
typedef Matrix<double, 3, 3> Matrix33d;

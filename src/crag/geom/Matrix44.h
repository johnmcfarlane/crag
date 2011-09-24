/*
 *  Matrix44.h
 *  Crag
 *
 *  Created by john on 6/30/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 */

#pragma once

#include "geom/Matrix.h"


// 4x4 matrix

// Column-major in line with OpenGL (and nothing else it would seem).
template<typename S> 
class Matrix<S, 4, 4>
{
public:
	// types
	typedef Vector<S, 4> Row;
	
	// functions
	Matrix()
	{
	}
	
	Matrix(Row const & row0,
			Row const & row1,
			Row const & row2,
			Row const & row3)
	{
		rows[0] = row0;
		rows[1] = row1;
		rows[2] = row2;
		rows[3] = row3;
	}
	
	Matrix(S m00, S m01, S m02, S m03, 
			S m10, S m11, S m12, S m13, 
			S m20, S m21, S m22, S m23, 
			S m30, S m31, S m32, S m33) 
	{
		rows[0][0] = m00;
		rows[0][1] = m01; 
		rows[0][2] = m02; 
		rows[0][3] = m03; 
		rows[1][0] = m10; 
		rows[1][1] = m11; 
		rows[1][2] = m12; 
		rows[1][3] = m13; 
		rows[2][0] = m20; 
		rows[2][1] = m21; 
		rows[2][2] = m22; 
		rows[2][3] = m23; 
		rows[3][0] = m30; 
		rows[3][1] = m31; 
		rows[3][2] = m32; 
		rows[3][3] = m33; 
	}
	
	friend inline bool operator == (Matrix const & lhs, Matrix const & rhs)
	{
		return lhs.rows[0] == rhs.rows[0]
		&& lhs.rows[1] == rhs.rows[1]
		&& lhs.rows[2] == rhs.rows[2]
		&& lhs.rows[3] == rhs.rows[3];
	}
	
	// Fast accessors - but behavior is different depending on whether matrix is row or column-major
	Row & operator [] (int i) 
	{
		assert(i >= 0 && i < 4);
		return rows[i];
	}
	
	Row const & operator [] (int i) const
	{
		assert(i >= 0 && i < 4);
		return rows[i];
	}
	
	// returns a raw array of numbers
	S * GetArray() 
	{
		return & rows[0][0];
	}
	
	S const * GetArray() const 
	{
		return & rows[0][0];
	}
	
	void SetArray(S const rhs [16])
	{
		BitwiseCopyArray(GetArray(), rhs, 16);
	}
	
	// fast because matrix is row major
	Row const & GetRow(int i) const 
	{ 
		assert(i >= 0 && i < 4);
		return rows[i]; 
	}
	
	// slow because matrix is row major and return value needs to be composed and copied
	Row GetColumn(int i) const 
	{ 
		assert(i >= 0 && i < 4);
		return Row(rows[0][i], rows[1][i], rows[2][i], rows[3][i]); 
	}
	
	S CoFactor(int row, int column) const
	{
		assert(row >= 0 && row < 4);
		assert(column >= 0 && column < 4);

		int row_1 = (row + 1) & 3;
		int row_2 = (row + 2) & 3;
		int row_3 = (row + 3) & 3;
		int column_1 = (column + 1) & 3;
		int column_2 = (column + 2) & 3;
		int column_3 = (column + 3) & 3;
		S a =	(	rows[row_1][column_1]*rows[row_2][column_2]*rows[row_3][column_3] +
					rows[row_1][column_2]*rows[row_2][column_3]*rows[row_3][column_1] +
					rows[row_1][column_3]*rows[row_2][column_1]*rows[row_3][column_2])
			  - (	rows[row_3][column_1]*rows[row_2][column_2]*rows[row_1][column_3] +
					rows[row_3][column_2]*rows[row_2][column_3]*rows[row_1][column_1] +
					rows[row_3][column_3]*rows[row_2][column_1]*rows[row_1][column_2]);
		
		if ((row + column) & 1) {
			a *= -1.f;
		}
		
		return a;
	}
	
	S Determinant() const
	{
		return (rows[0][0] * CoFactor(0, 0) + 
				rows[0][1] * CoFactor(0, 1) + 
				rows[0][2] * CoFactor(0, 2) +
				rows[0][3] * CoFactor(0, 3));
	}
	
	static Matrix Identity() 
	{
		return Matrix(Row(1, 0, 0, 0),
					  Row(0, 1, 0, 0),
					  Row(0, 0, 1, 0),
					  Row(0, 0, 0, 1)); 
	}
	
	static Matrix Zero() 
	{
		return Matrix(Row::Zero(), 
					  Row::Zero(), 
					  Row::Zero(), 
					  Row::Zero());
	}
	
private:

	// variables
	Row rows [4];
};


// single-precision floating-point specialization
typedef Matrix<float, 4, 4> Matrix44f;

// double-precision floating-point specialization
typedef Matrix<double, 4, 4> Matrix44d;
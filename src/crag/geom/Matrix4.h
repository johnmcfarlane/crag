/*
 *  Matrix4.h
 *  Crag
 *
 *  Created by john on 6/30/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *  Most of this file makes no sense and was adapted from ODE/OPCODE (http://www.codercorner.com/Opcode.htm)
 *
 */

#pragma once

#include "core/floatOps.h"
#include "geom/Vector4.h"


// 4x4 matrix

// Column-major in line with OpenGL (and nothing else it would seem).
template<typename S> 
class Matrix4
{
public:
	// types
	typedef Vector<S, 4> Row;
	
	// functions
	Matrix4()
	{
	}
	
	Matrix4(Row const & row0,
			Row const & row1,
			Row const & row2,
			Row const & row3)
	{
		rows[0] = row0;
		rows[1] = row1;
		rows[2] = row2;
		rows[3] = row3;
	}
	
	Matrix4(S m00, S m01, S m02, S m03, 
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
	
	friend inline bool operator == (Matrix4 const & lhs, Matrix4 const & rhs)
	{
		for (int i = 0; i < 16; ++ i)
		{
			if (lhs.rows[i] != rhs.rows[i])
			{
				return false;
			}
		}
		
		return true;
	}
	
	// Fast accessors - but behavior is different depending on whether matrix is row or column-major
	Row & operator [] (int i) 
	{
		return rows[i];
	}
	
	Row const & operator [] (int i) const
	{
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
		return rows[i]; 
	}
	
	// slow because matrix is row major and return value needs to be composed and copied
	Row GetColumn(int i) const 
	{ 
		return Row(rows[0][i], rows[1][i], rows[2][i], rows[3][i]); 
	}
	
	S CoFactor(int row, int column) const
	{
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
	
	static Matrix4 Identity() 
	{
		return Matrix4(Row(1, 0, 0, 0),
					   Row(0, 1, 0, 0),
					   Row(0, 0, 1, 0),
					   Row(0, 0, 0, 1)); 
	}
	
	static Matrix4 Zero() 
	{
		return Matrix4(Row::Zero(), 
					   Row::Zero(), 
					   Row::Zero(), 
					   Row::Zero());
	}
	
private:

	// variables
	Row rows [4];
};


template<typename S> 
Matrix4<S> operator * (Matrix4<S> const & lhs, Matrix4<S> const & rhs)
{
	Matrix4<S> result;
	result[0][0] = lhs[0][0]*rhs[0][0] + lhs[0][1]*rhs[1][0] + lhs[0][2]*rhs[2][0] + lhs[0][3]*rhs[3][0];
	result[0][1] = lhs[0][0]*rhs[0][1] + lhs[0][1]*rhs[1][1] + lhs[0][2]*rhs[2][1] + lhs[0][3]*rhs[3][1];
	result[0][2] = lhs[0][0]*rhs[0][2] + lhs[0][1]*rhs[1][2] + lhs[0][2]*rhs[2][2] + lhs[0][3]*rhs[3][2];
	result[0][3] = lhs[0][0]*rhs[0][3] + lhs[0][1]*rhs[1][3] + lhs[0][2]*rhs[2][3] + lhs[0][3]*rhs[3][3];
	result[1][0] = lhs[1][0]*rhs[0][0] + lhs[1][1]*rhs[1][0] + lhs[1][2]*rhs[2][0] + lhs[1][3]*rhs[3][0];
	result[1][1] = lhs[1][0]*rhs[0][1] + lhs[1][1]*rhs[1][1] + lhs[1][2]*rhs[2][1] + lhs[1][3]*rhs[3][1];
	result[1][2] = lhs[1][0]*rhs[0][2] + lhs[1][1]*rhs[1][2] + lhs[1][2]*rhs[2][2] + lhs[1][3]*rhs[3][2];
	result[1][3] = lhs[1][0]*rhs[0][3] + lhs[1][1]*rhs[1][3] + lhs[1][2]*rhs[2][3] + lhs[1][3]*rhs[3][3];
	result[2][0] = lhs[2][0]*rhs[0][0] + lhs[2][1]*rhs[1][0] + lhs[2][2]*rhs[2][0] + lhs[2][3]*rhs[3][0];
	result[2][1] = lhs[2][0]*rhs[0][1] + lhs[2][1]*rhs[1][1] + lhs[2][2]*rhs[2][1] + lhs[2][3]*rhs[3][1];
	result[2][2] = lhs[2][0]*rhs[0][2] + lhs[2][1]*rhs[1][2] + lhs[2][2]*rhs[2][2] + lhs[2][3]*rhs[3][2];
	result[2][3] = lhs[2][0]*rhs[0][3] + lhs[2][1]*rhs[1][3] + lhs[2][2]*rhs[2][3] + lhs[2][3]*rhs[3][3];
	result[3][0] = lhs[3][0]*rhs[0][0] + lhs[3][1]*rhs[1][0] + lhs[3][2]*rhs[2][0] + lhs[3][3]*rhs[3][0];
	result[3][1] = lhs[3][0]*rhs[0][1] + lhs[3][1]*rhs[1][1] + lhs[3][2]*rhs[2][1] + lhs[3][3]*rhs[3][1];
	result[3][2] = lhs[3][0]*rhs[0][2] + lhs[3][1]*rhs[1][2] + lhs[3][2]*rhs[2][2] + lhs[3][3]*rhs[3][2];
	result[3][3] = lhs[3][0]*rhs[0][3] + lhs[3][1]*rhs[1][3] + lhs[3][2]*rhs[2][3] + lhs[3][3]*rhs[3][3];
	return result;
}

template<typename S> 
Vector<S, 4> operator * (Matrix4<S> const & lhs, const Vector<S, 4> & rhs) 
{ 
	Vector<S, 4> result;
	result[0] = DotProduct(lhs.GetRow(0), rhs); 
	result[1] = DotProduct(lhs.GetRow(1), rhs); 
	result[2] = DotProduct(lhs.GetRow(2), rhs); 
	result[3] = DotProduct(lhs.GetRow(3), rhs); 
	return result;
}

template<typename S> 
Vector<S, 4> operator * (const Vector<S, 4> & lhs, Matrix4<S> const & rhs) 
{ 
	Vector<S, 4> result;
	result[0] = DotProduct(lhs, rhs.GetColumn(0)); 
	result[1] = DotProduct(lhs, rhs.GetColumn(1)); 
	result[2] = DotProduct(lhs, rhs.GetColumn(2)); 
	result[3] = DotProduct(lhs, rhs.GetColumn(3)); 
	return result;
}

template<typename S> 
S Determinant(Matrix4<S> const & matrix) 
{
	return matrix[0][0] * matrix.CoFactor(0, 0)
		+ matrix[0][1] * matrix.CoFactor(0, 1)
		+ matrix[0][2] * matrix.CoFactor(0, 2)
		+ matrix[0][3] * matrix.CoFactor(0, 3);
}

template<typename S> 
Matrix4<S> Inverse(Matrix4<S> const & matrix)
{
	Matrix4<S> inverse;
	
	S det = Determinant(matrix);
	if(Abs(det) < std::numeric_limits<S>::min()) 
	{
		return matrix;		// The matrix is not invertible! Singular case!
	}
	
	S inv_det = Inverse(det);
	
	inverse[0][0] = matrix.CoFactor(0,0) * inv_det;
	inverse[1][0] = matrix.CoFactor(0,1) * inv_det;
	inverse[2][0] = matrix.CoFactor(0,2) * inv_det;
	inverse[3][0] = matrix.CoFactor(0,3) * inv_det;
	inverse[0][1] = matrix.CoFactor(1,0) * inv_det;
	inverse[1][1] = matrix.CoFactor(1,1) * inv_det;
	inverse[2][1] = matrix.CoFactor(1,2) * inv_det;
	inverse[3][1] = matrix.CoFactor(1,3) * inv_det;
	inverse[0][2] = matrix.CoFactor(2,0) * inv_det;
	inverse[1][2] = matrix.CoFactor(2,1) * inv_det;
	inverse[2][2] = matrix.CoFactor(2,2) * inv_det;
	inverse[3][2] = matrix.CoFactor(2,3) * inv_det;
	inverse[0][3] = matrix.CoFactor(3,0) * inv_det;
	inverse[1][3] = matrix.CoFactor(3,1) * inv_det;
	inverse[2][3] = matrix.CoFactor(3,2) * inv_det;
	inverse[3][3] = matrix.CoFactor(3,3) * inv_det;
	
	return inverse;
}

template<typename S> 
Matrix4<S> & Transpose(Matrix4<S> & matrix)
{
	std::swap(matrix[1][0], matrix[0][1]);
	std::swap(matrix[2][0], matrix[0][2]);
	std::swap(matrix[2][1], matrix[1][2]);
	std::swap(matrix[3][0], matrix[0][3]);
	std::swap(matrix[3][1], matrix[1][3]);
	std::swap(matrix[3][2], matrix[2][3]);
	return matrix;
}

template<typename S> 
Matrix4<S> Transposition(Matrix4<S> const & matrix)
{
	Matrix4<S> transposition;
	
	for (int r = 0; r < 4; ++ r) {
		for (int c = 0; c < 4; ++ c) {
			transposition[c][r] = matrix[r][c];
		}
	}
	
	return transposition;
}

template<typename S> 
std::ostream & operator << (std::ostream & out, Matrix4<S> const & m)
{
	out << '[';
	for (int row = 0; row < 4; ++ row) {
		out << '[' << m[row][0] << ',' << m[row][1] << ',' << m[row][2] << ',' << m[row][3] << ']';
	}
	return out << ']';
}

/*template<typename S> 
std::istream & operator >> (std::istream & in, Matrix4<S> & m)
{
	in >> '[';
	for (int row = 0; row < 4; ++ row) {
		in >> '[' >> m[row][0] >> ',' >> m[row][1] >> ',' >> m[row][2] >> ',' >> m[row][3] >> ']';
	}
	return in >> ']';
}*/


// single-precision floating-point specialization
typedef Matrix4<float> Matrix4f;


// double-precision floating-point specialization
typedef Matrix4<double> Matrix4d;

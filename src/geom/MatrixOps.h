/*
 *  MatrixUtils.h
 *  Crag
 *
 *  Created by John on 12/9/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


////////////////////////////////////////////////////////////////////////////////
// Matrix multiplication

// matrix * matrix = matrix
template <typename S, int R, int C> 
Matrix<S, R, C> operator * (Matrix<S, R, C> const & lhs, Matrix<S, R, C> const & rhs)
{
	Matrix<S, R, C> result;
	for (int row = 0; row < R; ++ row)
	{
		for (int column = 0; column < C; ++ column)
		{
			result[row][column] = (lhs[row][0]*rhs[0][column] + 
								   lhs[row][1]*rhs[1][column] + 
								   lhs[row][2]*rhs[2][column] + 
								   lhs[row][3]*rhs[3][column]);
		}
	}
	return result;
}

// matrix * vector = vector
template<typename S, int R, int C> 
Vector<S, R> operator * (Matrix<S, R, C> const & lhs, const Vector<S, C> & rhs) 
{ 
	Vector<S, R> result;
	for (int row = 0; row < R; ++ row)
	{
		result[row] = DotProduct(lhs.GetRow(row), rhs);
	}
	return result;
}

// vector * matrix = vector
template<typename S, int R, int C> 
Vector<S, C> operator * (const Vector<S, R> & lhs, Matrix<S, R, C> const & rhs) 
{ 
	Vector<S, C> result;
	for (int column = 0; column < C; ++ column)
	{
		result[column] = DotProduct(lhs, rhs.GetColumn(column)); 
	}
	return result;
}


////////////////////////////////////////////////////////////////////////////////
// Miscellaneous Matrix operations

template<typename S, int R, int C> 
Matrix<S, R, C> Transposition(Matrix<S, R, C> const & matrix)
{
	Matrix<S, R, C> transposition;
	
	for (int row = 0; row < R; ++ row) 
	{
		for (int column = 0; column < C; ++ column) 
		{
			transposition[column][row] = matrix[row][column];
		}
	}
	
	return transposition;
}


////////////////////////////////////////////////////////////////////////////////
// I/O

template <typename S, int R, int C> 
std::ostream & operator << (std::ostream & out, Matrix<S, R, C> const & m)
{
	out << '[';
	for (int row = 0; row < R; ++ row) 
	{
		out << '[';
		for (int column = 0; column < C; ++ column) 
		{
			out << m[row][column] << ((column == (C - 1)) ? ']' : ',');
		}
	}
	return out << ']';
}

template <typename S, int R, int C> 
std::istream & operator >> (std::istream & in, Matrix<S, R, C> & m)
{
	char dummy;
	in >> dummy;
	for (int row = 0; row < R; ++ row) 
	{
		in >> dummy;
		for (int column = 0; column < C; ++ column) 
		{
			in >> m[row][column] >> dummy;
		}
	}
	return in >> dummy;
}

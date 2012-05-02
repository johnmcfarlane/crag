//
//  MatrixOps.h
//  crag
//
//  Created by John on 12/9/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace geom
{
	////////////////////////////////////////////////////////////////////////////////
	// Matrix multiplication

	// matrix * matrix = matrix
	template <typename S, int N> 
	Matrix<S, N, N> operator * (Matrix<S, N, N> const & lhs, Matrix<S, N, N> const & rhs)
	{
		S result[N][N];
		for (int row = 0; row != N; ++ row)
		{
			for (int column = 0; column != N; ++ column)
			{
				S cell = 0;
				for (int i = 0; i != N; ++ i)
				{
					cell += lhs[row][i] * rhs[i][column];
				}
				result[row][column] = cell;
			}
		}
		return result;
	}

	// matrix * vector = vector
	template<typename S, int R, int C> 
	Vector<S, R> operator * (Matrix<S, R, C> const & lhs, Vector<S, C> const & rhs) 
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
	Vector<S, C> operator * (Vector<S, R> const & lhs, Matrix<S, R, C> const & rhs) 
	{ 
		Vector<S, C> result;
		for (int column = 0; column < C; ++ column)
		{
			result[column] = DotProduct(lhs, rhs.GetColumn(column)); 
		}
		return result;
	}


	////////////////////////////////////////////////////////////////////////////////
	// Matrix relation operators

	template<typename S, int R, int C> 
	bool operator == (Matrix<S, R, C> const & lhs, Matrix<S, R, C> const & rhs) 
	{ 
		for (int row = 0; row < R; ++ row)
		{
			for (int column = 0; column < C; ++ column)
			{
				if (lhs[row][column] != rhs[row][column])
				{
					return false;
				}
			}
		}
			
		return true;
	}

	template<typename S, int R, int C> 
	bool operator != (Matrix<S, R, C> const & lhs, Matrix<S, R, C> const & rhs) 
	{
		return ! (lhs == rhs);
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
}

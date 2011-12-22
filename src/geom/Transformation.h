//
//  Transformation.h
//  crag
//
//  Created by John McFarlane on 9/22/11.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Matrix33.h"
#include "Matrix44.h"
#include "MatrixOps.h"


// Affine transformation matrix
template<typename S>
class Transformation
{
public:
	////////////////////////////////////////////////////////////////////////////////
	// types
	
	typedef Matrix<S, 3, 3> Rotation;
	typedef Matrix<S, 4, 4> Matrix;
	typedef Vector<S, 3> Vector;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions 
	
	Transformation()
	{
	}
	
	Transformation(Vector const & translation)
	: _matrix(1, 0, 0, translation.x,
			  0, 1, 0, translation.y, 
			  0, 0, 1, translation.z,
			  0, 0, 0, 1)
	{
	}
	
	Transformation(Vector const & translation, Rotation const & rotation)
	: _matrix(rotation[0][0], rotation[0][1], rotation[0][2], translation[0],
			  rotation[1][0], rotation[1][1], rotation[1][2], translation[1],
			  rotation[2][0], rotation[2][1], rotation[2][2], translation[2],
			  0, 0, 0, 1)
	{
	}
	
	Transformation(Vector const & translation, Rotation const & rotation, S scale)
	: _matrix(rotation[0][0] * scale, rotation[0][1] * scale, rotation[0][2] * scale, translation[0],
			  rotation[1][0] * scale, rotation[1][1] * scale, rotation[1][2] * scale, translation[1],
			  rotation[2][0] * scale, rotation[2][1] * scale, rotation[2][2] * scale, translation[2],
			  0, 0, 0, 1)
	{
	}
	
	Transformation(Vector const & translation, Rotation const & rotation, Vector const & scale)
	: _matrix(rotation[0][0] * scale[0], rotation[0][1] * scale[1], rotation[0][2] * scale[2], translation[0],
			  rotation[1][0] * scale[0], rotation[1][1] * scale[1], rotation[1][2] * scale[2], translation[1],
			  rotation[2][0] * scale[0], rotation[2][1] * scale[1], rotation[2][2] * scale[2], translation[2],
			  0, 0, 0, 1)
	{
	}
	
	Transformation(Matrix const & matrix)
	: _matrix(matrix)
	{
	}
	
	Transformation & operator=(Transformation const & rhs)
	{
		_matrix = rhs._matrix;
		return * this;
	}
	
	friend Transformation operator * (Transformation const & lhs, Transformation const & rhs)
	{
		return Transformation(lhs._matrix * rhs._matrix);
	}
	
	friend bool operator == (Transformation const & lhs, Transformation const & rhs)
	{
		return lhs._matrix == rhs._matrix;
	}
	
	friend bool operator != (Transformation const & lhs, Transformation const & rhs)
	{
		return lhs._matrix != rhs._matrix;
	}
	
	// return the translation from the transformation
	Matrix const & GetMatrix() const
	{
		return _matrix;
	}
	
	Matrix GetInverse() const
	{
		return Inverse(_matrix);
	}
	
	Vector GetTranslation() const
	{
		return Vector(_matrix[0][3], _matrix[1][3], _matrix[2][3]);
	}
	
	Rotation const & GetRotation() const
	{
		return reinterpret_cast<Rotation const &>(* this);
	}
	
	Vector GetScale() const
	{
		Rotation const & rotation = GetRotation();
		return Vector(Length(rotation.GetColumn(0)), 
					  Length(rotation.GetColumn(1)), 
					  Length(rotation.GetColumn(2)));
	}

private:
	// variables
	Matrix _matrix;
};

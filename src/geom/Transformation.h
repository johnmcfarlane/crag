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


namespace geom
{
	// Affine transformation matrix
	template <typename S>
	class Transformation
	{
	public:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef S Scalar;
		typedef Matrix<Scalar, 3, 3> Matrix33;
		typedef Matrix<Scalar, 4, 4> Matrix44;
		typedef Ray<Scalar, 3> Ray3;
		typedef Vector<Scalar, 3> Vector3;
		typedef Vector<Scalar, 4> Vector4;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions 
		
		Transformation()
		: Transformation(Matrix44::Identity())
		{
		}
		
		Transformation(Transformation const & rhs)
		: _matrix(rhs.GetMatrix())
		{
		}

		Transformation(Vector3 const & translation)
		: _matrix(1, 0, 0, translation.x,
				  0, 1, 0, translation.y, 
				  0, 0, 1, translation.z,
				  0, 0, 0, 1)
		{
		}

		Transformation(Vector3 const & translation, Matrix33 const & rotation)
		: _matrix(rotation[0][0], rotation[0][1], rotation[0][2], translation[0],
				  rotation[1][0], rotation[1][1], rotation[1][2], translation[1],
				  rotation[2][0], rotation[2][1], rotation[2][2], translation[2],
				  0, 0, 0, 1)
		{
		}
		
		Transformation(Vector3 const & translation, Matrix33 const & rotation, Scalar scale)
		: _matrix(rotation[0][0] * scale, rotation[0][1] * scale, rotation[0][2] * scale, translation[0],
				  rotation[1][0] * scale, rotation[1][1] * scale, rotation[1][2] * scale, translation[1],
				  rotation[2][0] * scale, rotation[2][1] * scale, rotation[2][2] * scale, translation[2],
				  0, 0, 0, 1)
		{
		}
		
		Transformation(Vector3 const & translation, Matrix33 const & rotation, Vector3 const & scale)
		: _matrix(rotation[0][0] * scale[0], rotation[0][1] * scale[1], rotation[0][2] * scale[2], translation[0],
				  rotation[1][0] * scale[0], rotation[1][1] * scale[1], rotation[1][2] * scale[2], translation[1],
				  rotation[2][0] * scale[0], rotation[2][1] * scale[1], rotation[2][2] * scale[2], translation[2],
				  0, 0, 0, 1)
		{
		}
		
		Transformation(Matrix44 const & matrix)
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
		
		Matrix44 & GetMatrix()
		{
			return _matrix;
		}
		
		Matrix44 const & GetMatrix() const
		{
			return _matrix;
		}

		Vector3 GetTranslation() const
		{
			return Vector3(_matrix[0][3], _matrix[1][3], _matrix[2][3]);
		}
		
		Vector4 GetTranslation4() const
		{
			return Vector4(_matrix.GetColumn(3));
		}
		
		void SetTranslation(Vector3 const & translation)
		{
			_matrix[0][3] = translation.x;
			_matrix[1][3] = translation.y;
			_matrix[2][3] = translation.z;
		}
		
		Matrix33 const & GetRotation() const
		{
			return reinterpret_cast<Matrix33 const &>(* this);
		}
		
		void SetRotation(Matrix33 const & rotation)
		{
			for (auto r = 0; r != 3; ++ r)
			{
				for (auto c = 0; c != 3; ++ c)
				{
					_matrix[r][c] = rotation[r][c];
				}
			}
		}
		
		Vector3 GetScale() const
		{
			Matrix33 const & rotation = GetRotation();
			return Vector3(Magnitude(rotation.GetColumn(0)),
				Magnitude(rotation.GetColumn(1)),
				Magnitude(rotation.GetColumn(2)));
		}
		
		Vector3 Transform(Vector3 const & point) const
		{
			Vector4 v(point.x, point.y, point.z, Scalar(1));
			v = _matrix * v;
			return Vector3(v.x, v.y, v.z);
		}
		
		Vector3 Rotate(Vector3 const & direction) const
		{
			return GetRotation() * direction;
		}
		
		Ray3 Transform(Ray3 const & ray) const
		{
			return Ray3(Transform(ray.position), Rotate(ray.direction));
		}
		
#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(Transformation, self)
			CRAG_VERIFY(self._matrix);
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif

	private:
		// variables
		Matrix44 _matrix;
	};

	template <typename S>
	Transformation<S> Inverse(Transformation<S> const & transformation)
	{
		return ::geom::Inverse(transformation.GetMatrix());
	}
}

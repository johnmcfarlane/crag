//
//  Space.h
//  crag
//
//  Created by John on 2012-12-23.
//  Copyright 2009-2014 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Matrix33.h"
#include "Matrix44.h"
#include "Transformation.h"

namespace geom
{
	// relative space types
	template <typename SCALAR>
	struct Precision
	{
		typedef SCALAR Scalar;
		typedef Vector<Scalar, 2> Vector2;
		typedef Vector<Scalar, 3> Vector3;
		typedef Ray<Scalar, 3> Ray3;
		typedef Sphere<Scalar, 3> Sphere3;
		typedef Matrix<Scalar, 3, 3> Matrix33;
		typedef Matrix<Scalar, 4, 4> Matrix44;
		typedef Transformation<Scalar> Transformation;
	};

	using rel = Precision<float>;
	using abs = Precision<double>;

	////////////////////////////////////////////////////////////////////////////////
	// Space class definition
	
	class Space;

	template <typename REL_T>
	REL_T Convert(REL_T const & rel, Space const & source, Space const & destination);
	
	// a relative co-ordinate space; helps maintain precision
	// when using single-precision floats far from the absolute origin
	class Space
	{
		////////////////////////////////////////////////////////////////////////////////
		// types
	
		template <typename REL_T>
		friend REL_T Convert(REL_T const & rel, Space const & source, Space const & destination);

		using OriginType = abs::Vector3;

	public:
		////////////////////////////////////////////////////////////////////////////////
		// functions

		// operators
		Space() = default;

		Space(Space const &) = default;
		
		Space(OriginType const & _origin)
		: _origin(_origin)
		{
			ASSERT(_origin == _origin);
		}

		Space & operator=(Space const &) = default;
		
		// access
		OriginType const & GetOrigin() const
		{
			return _origin;
		}

		void SetOrigin(OriginType const & origin)
		{
			_origin = origin;
		}

		// AbsToRel
		template <typename REL_S = rel::Scalar>
		Vector<REL_S, 3> AbsToRel(abs::Vector3 const & abs) const
		{
			return Cast<REL_S>(abs - _origin);
		}
	
		template <typename REL_S = rel::Scalar>
		Ray<REL_S, 3> AbsToRel(abs::Ray3 const & abs) const
		{
			return Ray<REL_S, 3>(AbsToRel<REL_S>(abs.position), Cast<REL_S>(abs.direction));
		}
	
		template <typename REL_S = rel::Scalar>
		Sphere<REL_S, 3> AbsToRel(abs::Sphere3 const & abs) const
		{
			return Sphere<REL_S, 3>(AbsToRel<REL_S>(abs.center), static_cast<REL_S>(abs.radius));
		}
	
		template <typename REL_S = rel::Scalar>
		Transformation<REL_S> AbsToRel(abs::Transformation const & abs) const
		{
			return Transformation<REL_S>(AbsToRel<REL_S>(abs.GetTranslation()), Cast<REL_S>(abs.GetRotation()));
		}
	
		// RelToAbs
		template <typename ABS_S = abs::Scalar, typename REL_S = rel::Scalar>
		Vector<ABS_S, 3> RelToAbs(Vector<REL_S, 3> const & rel) const
		{
			return Cast<ABS_S>(rel) + Cast<ABS_S>(_origin);
		}
	
		template <typename ABS_S = abs::Scalar, typename REL_S = rel::Scalar>
		Ray<ABS_S, 3> RelToAbs(Ray<REL_S, 3> const & rel) const
		{
			return Ray<ABS_S, 3>(
				RelToAbs<ABS_S, REL_S>(rel.position), 
				Cast<ABS_S>(rel.direction));
		}
	
		template <typename ABS_S = abs::Scalar, typename REL_S = rel::Scalar>
		Sphere<ABS_S, 3> RelToAbs(Sphere<REL_S, 3> const & rel) const
		{
			return Sphere<ABS_S, 3>(
				RelToAbs<ABS_S, REL_S>(rel.center), 
				static_cast<ABS_S>(rel.radius));
		}
	
		template <typename ABS_S = abs::Scalar, typename REL_S = rel::Scalar>
		Transformation<ABS_S> RelToAbs(Transformation<REL_S> const & rel) const
		{
			return Transformation<ABS_S>(
				RelToAbs<ABS_S, REL_S>(rel.GetTranslation()), 
				Cast<ABS_S>(rel.GetRotation()));
		}

		// verification
		CRAG_VERIFY_INVARIANTS_DECLARE(Space);

	private:
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		// the relative origin against which relative coordinates are offset
		OriginType _origin = OriginType::Zero();
	};

	////////////////////////////////////////////////////////////////////////////////
	// geom::Space and helper definitions
	
	inline abs::Vector3 operator-(Space const & lhs, Space const & rhs)
	{
		return lhs.GetOrigin() - rhs.GetOrigin();
	}

#if defined(CRAG_VERIFY_ENABLED)
	inline CRAG_VERIFY_INVARIANTS_DEFINE_BEGIN(Space, self)
		CRAG_VERIFY(self._origin);
	CRAG_VERIFY_INVARIANTS_DEFINE_END
#endif

	template <typename REL_T>
	REL_T Convert(REL_T const & rel, Space const & source, Space const & destination)
	{
		typedef typename REL_T::Scalar Scalar;
		auto abs = source.RelToAbs(rel);
		return destination.AbsToRel<Scalar>(abs);
	}
}

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
		typedef geom::Transformation<Scalar> Transformation;
	};

	using rel = Precision<float>;
	using uni = Precision<double>;

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

		using OriginType = uni::Vector3;

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
		auto constexpr AbsToRel(uni::Vector3 const & uni) const noexcept {
			return static_cast<Vector<REL_S, 3>>(uni - _origin);
		}
	
		template <typename REL_S = rel::Scalar>
		Ray<REL_S, 3> AbsToRel(uni::Ray3 const & uni) const
		{
			return Ray<REL_S, 3>(
				AbsToRel<REL_S>(uni.position),
				static_cast<Vector<REL_S, 3>>(uni.direction));
		}
	
		template <typename REL_S = rel::Scalar>
		Sphere<REL_S, 3> AbsToRel(uni::Sphere3 const & uni) const
		{
			return Sphere<REL_S, 3>(
				AbsToRel<REL_S>(uni.center),
				static_cast<REL_S>(uni.radius));
		}
	
		template <typename REL_S = rel::Scalar>
		Transformation<REL_S> AbsToRel(uni::Transformation const & uni) const
		{
			return Transformation<REL_S>(
				AbsToRel<REL_S>(uni.GetTranslation()),
				static_cast<Matrix<REL_S, 3, 3>>(uni.GetRotation()));
		}
	
		// RelToAbs
		template <typename ABS_S = uni::Scalar, typename REL_S = rel::Scalar>
		auto constexpr RelToAbs(Vector<REL_S, 3> const & rel) const noexcept {
			using Result = Vector<ABS_S, 3>;
			return static_cast<Result>(rel) + static_cast<Result>(_origin);
		}
	
		template <typename ABS_S = uni::Scalar, typename REL_S = rel::Scalar>
		Ray<ABS_S, 3> RelToAbs(Ray<REL_S, 3> const & rel) const
		{
			return Ray<ABS_S, 3>(
				RelToAbs<ABS_S, REL_S>(rel.position),
				static_cast<Vector<ABS_S, 3>>(rel.direction));
		}
	
		template <typename ABS_S = uni::Scalar, typename REL_S = rel::Scalar>
		Sphere<ABS_S, 3> RelToAbs(Sphere<REL_S, 3> const & rel) const
		{
			return Sphere<ABS_S, 3>(
				RelToAbs<ABS_S, REL_S>(rel.center), 
				static_cast<ABS_S>(rel.radius));
		}
	
		template <typename ABS_S = uni::Scalar, typename REL_S = rel::Scalar>
		Transformation<ABS_S> RelToAbs(Transformation<REL_S> const & rel) const
		{
			using Transformation = Transformation<ABS_S>;
			using Matrix = typename Transformation::Matrix33;
			return Transformation(
				RelToAbs<ABS_S, REL_S>(rel.GetTranslation()),
				static_cast<Matrix>(rel.GetRotation()));
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
	
	inline uni::Vector3 operator-(Space const & lhs, Space const & rhs)
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
		auto uni = source.RelToAbs(rel);
		return destination.AbsToRel<Scalar>(uni);
	}
}

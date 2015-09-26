//
//  core/unit_interval.h
//  crag
//
//  Created by John McFarlane on 2015-09-08.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <fixed_point.h>

namespace crag
{
	namespace core
	{
		// numeric type which uses the full range of an underlying signed or unsigned
		// integer type, ReprType, to represent a value in the range [-1, 1] or [0, 1]
		// respectively
		template <class ReprType>
		class unit_interval
		{
		public:
			////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////
			// types

			using repr_type = ReprType;

			////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////
			// functions
		private:

			////////////////////////////////////////////////////////////////////////////////
			// constructors / type operators

			constexpr unit_interval(const ReprType & repr, int)
				: _repr(repr)
			{
			}

		public:
			template <class Input>
			explicit constexpr unit_interval(const Input & input) noexcept
				: _repr(to_repr(input))
			{
			}

			// convert to integer type (using full range of the type)
			template <class Output, typename std::enable_if<_impl::is_integral<Output>::value, int>::type _Dummy = 0>
			constexpr Output integral() const noexcept
			{
				using intermediate_type = sg14::_impl::next_size_t<typename std::common_type<Output, ReprType>::type>;
				return intermediate_type{_repr} * std::numeric_limits<Output>::max() / _max;
			}

			// convert to floating-point type
			template <class Output, typename std::enable_if<std::is_floating_point<Output>::value, int>::type _Dummy = 0>
			explicit constexpr operator Output() const noexcept
			{
				return static_cast<Output>(_repr) / _max;
			}

			////////////////////////////////////////////////////////////////////////////////
			// leaks

			constexpr const ReprType & data() const noexcept
			{
				return _repr;
			}

			static constexpr unit_interval from_data(const ReprType & repr) noexcept
			{
				return unit_interval(repr, 0);
			}

			////////////////////////////////////////////////////////////////////////////////
			// operator overloads

			template <class Rhs>
			constexpr friend auto operator==(const unit_interval & lhs, const Rhs & rhs) noexcept
			-> typename std::enable_if<std::is_integral<Rhs>::value, bool>::type
			{
				return static_cast<Rhs>(lhs) == rhs;
			}

		private:
			template <class Input>
			static constexpr ReprType to_repr(const Input & input)
			{
				// unless the type of the given floating-point value has
				// higher capacity than unit_interval's underlying representation,
				// then some inputs will fail, e.g. 1
				static_assert(! std::is_floating_point<Input>::value || sizeof(Input) > sizeof(ReprType), "unreliable input type");

				return /*(input < Input(0) || input > Input(1))
					   ? throw std::out_of_range("unit_interval can only be assigned a value in the range [0, 1]")
					   : */input * _max;
			}

			////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////
			// constants

			static constexpr ReprType _max = std::numeric_limits<ReprType>::max();

			////////////////////////////////////////////////////////////////////////////////
			////////////////////////////////////////////////////////////////////////////////
			// variables

			ReprType _repr;
		};

		////////////////////////////////////////////////////////////////////////////////
		// operators

		template <class ReprType>
		unit_interval<ReprType> operator +(
			const unit_interval<ReprType> & lhs,
			const unit_interval<ReprType> & rhs)
		{
			return unit_interval<ReprType>::from_data(lhs.data() + rhs.data());
		}

		template <class ReprType>
		unit_interval<ReprType> operator -(
			const unit_interval<ReprType> & lhs,
			const unit_interval<ReprType> & rhs)
		{
			return unit_interval<ReprType>::from_data(lhs.data() - rhs.data());
		}

		template <class ReprType>
		unit_interval<ReprType> operator *(
			const unit_interval<ReprType> & lhs,
			const unit_interval<ReprType> & rhs)
		{
			return unit_interval<ReprType>::from_data(lhs.data() * rhs.data());
		}
	}
}

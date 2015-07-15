//
//  core/fixed_point.h
//  crag
//
//  Created by John McFarlane on 2015-07-08.
//  Copyright 2015 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <climits>
#include <cmath>
#include <cinttypes>
#include <limits>
#include <type_traits>

namespace crag
{
	namespace core
	{
		////////////////////////////////////////////////////////////////////////////////
		// forward-declarations

		namespace _impl
		{
			// given an integral (except bool) type T, provides the member typedef type
			// which is the next longest sized integer type corresponding to T
			template <typename INT_TYPE>
			struct next_size;

			template <typename T>
			using next_size_t = typename next_size<T>::type;
			// performs a shift operation by a fixed number of bits avoiding two pitfalls:
			// 1) shifting by a negative amount causes undefined behavior
			// 2) converting between integer types of different sizes can lose significant bits during shift right
			template <int EXPONENT, typename OUTPUT, typename INPUT, typename std::enable_if<EXPONENT >= 0 && sizeof(OUTPUT) <= sizeof(INPUT), int>::type dummy = 0>
			constexpr OUTPUT shift_left(INPUT i) noexcept
			{
				static_assert(std::is_integral<INPUT>::value, "INPUT must be integral type");
				static_assert(std::is_integral<OUTPUT>::value, "OUTPUT must be integral type");
				return static_cast<OUTPUT>(i) << EXPONENT;
			}

			template <int EXPONENT, typename OUTPUT, typename INPUT, typename std::enable_if<EXPONENT >= 0 && ! (sizeof(OUTPUT) <= sizeof(INPUT)), char>::type dummy = 0>
			constexpr OUTPUT shift_left(INPUT i) noexcept
			{
				static_assert(std::is_integral<INPUT>::value, "INPUT must be integral type");
				static_assert(std::is_integral<OUTPUT>::value, "OUTPUT must be integral type");
				return static_cast<OUTPUT>(i << EXPONENT);
			}

			template <int EXPONENT, typename OUTPUT, typename INPUT, typename std::enable_if<EXPONENT >= 0 && sizeof(OUTPUT) <= sizeof(INPUT), int>::type dummy = 0>
			constexpr OUTPUT shift_right(INPUT i) noexcept
			{
				static_assert(std::is_integral<INPUT>::value, "INPUT must be integral type");
				static_assert(std::is_integral<OUTPUT>::value, "OUTPUT must be integral type");
				return static_cast<OUTPUT>(i >> EXPONENT);
			}

			template <int EXPONENT, typename OUTPUT, typename INPUT, typename std::enable_if<EXPONENT >= 0 && !(sizeof(OUTPUT) <= sizeof(INPUT)), char>::type dummy = 0>
			constexpr OUTPUT shift_right(INPUT i) noexcept
			{
				static_assert(std::is_integral<INPUT>::value, "INPUT must be integral type");
				static_assert(std::is_integral<OUTPUT>::value, "OUTPUT must be integral type");
				return static_cast<OUTPUT>(i) >> EXPONENT;
			}

			template <int EXPONENT, typename OUTPUT, typename INPUT, typename std::enable_if<(EXPONENT < 0), int>::type dummy = 0>
			constexpr OUTPUT shift_left(INPUT i) noexcept
			{
				return shift_right<-EXPONENT, OUTPUT, INPUT>(i);
			}

			template <int EXPONENT, typename OUTPUT, typename INPUT, typename std::enable_if<EXPONENT < 0, int>::type dummy = 0>
			constexpr OUTPUT shift_right(INPUT i) noexcept
			{
				return shift_left<-EXPONENT, OUTPUT, INPUT>(i);
			}

			// returns given power of 2
			template <typename S, int EXPONENT, typename std::enable_if<EXPONENT == 0, int>::type dummy = 0>
			constexpr S pow2() noexcept
			{
				static_assert(std::is_floating_point<S>::value, "S must be floating-point type");
				return 1;
			}

			template <typename S, int EXPONENT, typename std::enable_if<!(EXPONENT <= 0), int>::type dummy = 0>
			constexpr S pow2() noexcept
			{
				static_assert(std::is_floating_point<S>::value, "S must be floating-point type");
				return pow2<S, EXPONENT - 1>() * S(2);
			}

			template <typename S, int EXPONENT, typename std::enable_if<!(EXPONENT >= 0), int>::type dummy = 0>
			constexpr S pow2() noexcept
			{
				static_assert(std::is_floating_point<S>::value, "S must be floating-point type");
				return pow2<S, EXPONENT + 1>() * S(.5);
			}
		}

		template <typename REPR_TYPE, int EXPONENT>
		class fixed_point;

		// fixed-point type capable of storing values in the range [0, 1);
		// a bit more precise than closed_unit
		template <typename REPR_TYPE>
		using open_unit = fixed_point<REPR_TYPE, - static_cast<int>(sizeof(REPR_TYPE)) * CHAR_BIT>;

		// fixed-point type capable of storing values in the range [0, 1];
		// actually storing values in the range [0, 2);
		// a bit less precise than closed_unit
		template <typename REPR_TYPE>
		using closed_unit = fixed_point<REPR_TYPE, 1 - static_cast<int>(sizeof(REPR_TYPE)) * CHAR_BIT>;

		template <typename REPR_TYPE, int EXPONENT, typename S, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
		fixed_point<REPR_TYPE, EXPONENT> lerp(
			fixed_point<REPR_TYPE, EXPONENT> from,
			fixed_point<REPR_TYPE, EXPONENT> to,
			S t);

		template <typename REPR_TYPE, int EXPONENT>
		fixed_point<REPR_TYPE, EXPONENT> lerp(
			fixed_point<REPR_TYPE, EXPONENT> from,
			fixed_point<REPR_TYPE, EXPONENT> to,
			closed_unit<REPR_TYPE> t);

		////////////////////////////////////////////////////////////////////////////////
		// fixed_point class template definition
		//
		// approximates a real number using a machine word - much like a floating-point
		// number but - with exponent determined at run-time
		//
		// TODO: negative values, tests

		template <typename REPR_TYPE, int EXPONENT = 0>
		class fixed_point
		{
			// friends
			template <typename S, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy>
			friend fixed_point lerp(
				fixed_point<REPR_TYPE, EXPONENT> from,
				fixed_point<REPR_TYPE, EXPONENT> to,
				S t);

			friend fixed_point<REPR_TYPE, EXPONENT> lerp<REPR_TYPE, EXPONENT>(
				fixed_point<REPR_TYPE, EXPONENT> from,
				fixed_point<REPR_TYPE, EXPONENT> to,
				closed_unit<REPR_TYPE> t);

		public:
			// types
			using repr_type = REPR_TYPE;

			// constants
			constexpr static int exponent = EXPONENT;
			constexpr static repr_type repr_max = std::numeric_limits<repr_type>::max();
			constexpr static std::size_t repr_size = sizeof(repr_type);
			constexpr static std::size_t repr_num_bits = repr_size * CHAR_BIT;

			// functions
			constexpr fixed_point() noexcept {}

			template <typename S, typename std::enable_if<std::is_integral<S>::value, int>::type dummy = 0>
			constexpr fixed_point(S s) noexcept
				: _repr(int_to_repr(s))
			{
			}

			template <typename S, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
			constexpr fixed_point(S s) noexcept
				: _repr(float_to_repr(s))
			{
			}

			// returns value represented as a floating-point
			template <typename S, typename std::enable_if<std::is_integral<S>::value, int>::type dummy = 0>
			constexpr S get() const noexcept
			{
				return repr_to_int<S>(_repr);
			}

			// returns value represented as integral
			template <typename S, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
			constexpr S get() const noexcept
			{
				return repr_to_float<S>(_repr);
			}

			// returns internal representation of value
			constexpr repr_type data() const noexcept
			{
				return _repr;
			}

			// creates an instance given the underlying representation value
			static /*constexpr*/ fixed_point from_data(repr_type repr) noexcept
			{
				fixed_point fp;
				fp._repr = repr;
				return fp;
			}

		private:
			template <typename S, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
			static constexpr S one() noexcept
			{
				return std::pow(S(.5), EXPONENT);
			}

			template <typename S, typename std::enable_if<std::is_integral<S>::value, int>::type dummy = 0>
			static constexpr S one() noexcept
			{
				return int_to_repr(1);
			}

			template <typename S>
			static constexpr S inverse_one() noexcept
			{
				static_assert(std::is_floating_point<S>::value, "S must be floating-point type");
				return std::pow(S(.5), - EXPONENT);
			}

			template <typename S>
			static constexpr S int_to_repr(S s) noexcept
			{
				static_assert(std::is_integral<S>::value, "S must be unsigned integral type");

				return (EXPONENT > 0) ? s >> EXPONENT : s << (- EXPONENT);
			}

			template <typename S>
			static constexpr S repr_to_int(repr_type r) noexcept
			{
				static_assert(std::is_integral<S>::value, "S must be unsigned integral type");

				return (EXPONENT > 0) ? r << EXPONENT : r >> (- EXPONENT);
			}

			template <typename S>
			static constexpr S float_to_repr(S s) noexcept
			{
				static_assert(std::is_floating_point<S>::value, "S must be floating-point type");
				return s * one<S>();
			}

			template <typename S>
			static constexpr S repr_to_float(repr_type r) noexcept
			{
				static_assert(std::is_floating_point<S>::value, "S must be floating-point type");
				return S(r) * inverse_one<S>();
			}

			// variables
			repr_type _repr;
		};

		////////////////////////////////////////////////////////////////////////////////
		// helper definitions

		namespace _impl
		{
			// specializations of next_size<>
			template <> struct next_size<std::uint8_t> { using type = std::uint16_t; };
			template <> struct next_size<std::uint16_t> { using type = std::uint32_t; };
			template <> struct next_size<std::uint32_t> { using type = std::uint64_t; };
		}

		// linear interpolation between two identical specializations of fixed_point
		// given floatint-point `t` for which result is `from` when t==0 and `to` when t==1
		template <typename REPR_TYPE, int EXPONENT, typename S, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy>
		fixed_point<REPR_TYPE, EXPONENT> lerp(
			fixed_point<REPR_TYPE, EXPONENT> from,
			fixed_point<REPR_TYPE, EXPONENT> to,
			S t)
		{
			using closed_unit = closed_unit<REPR_TYPE>;
			return lerp<REPR_TYPE, EXPONENT>(from, to, closed_unit(t));
		}

		// linear interpolation between two identical specializations of fixed_point
		// given closed_unit `t` for which result is `from` when t==0 and `to` when t==1
		template <typename REPR_TYPE, int EXPONENT>
		fixed_point<REPR_TYPE, EXPONENT> lerp(
			fixed_point<REPR_TYPE, EXPONENT> from,
			fixed_point<REPR_TYPE, EXPONENT> to,
			closed_unit<REPR_TYPE> t)
		{
			using fixed_point = fixed_point<REPR_TYPE, EXPONENT>;
			using repr_type = typename fixed_point::repr_type;
			using next_repr_type = typename _impl::next_size_t<repr_type>;
			using closed_unit = closed_unit<REPR_TYPE>;

			constexpr auto _one = closed_unit::template one<REPR_TYPE>();
			auto _t = t.data();
			auto _t_neg = _one - _t;

			auto _from = static_cast<next_repr_type>(from._repr) * _t_neg;
			auto _to = static_cast<next_repr_type>(to._repr) * _t;
			auto sum = _from + _to;

			auto result = fixed_point();
			result._repr = (closed_unit::exponent > 0)
				? static_cast<repr_type>(sum << closed_unit::exponent)
				: static_cast<repr_type>(sum >> - closed_unit::exponent);

			return result;
		}
	}
}

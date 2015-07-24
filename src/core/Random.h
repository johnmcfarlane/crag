//
//  Random.h
//  crag
//
//  Created by john on 5/30/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

class Random
{
public:
	////////////////////////////////////////////////////////////////////////////
	// types
	
	using generator_type = std::minstd_rand;
	using result_type = generator_type::result_type;

	////////////////////////////////////////////////////////////////////////////
	// functions
	
	Random(result_type seed = 1) : generator(seed) { }
	
	// returns pseudo-random whole number in the range [0, maximum]
	result_type GetInt()
	{
		return generator();
	}
	
	// returns pseudo-random whole number in the range [0, limit)
	result_type GetInt(result_type limit)
	{
		uint64_t r = GetInt();
		r *= limit;
		r /= bound;

		CRAG_VERIFY_OP(r, >=, 0U);
		CRAG_VERIFY_OP(r, <, limit);

		return static_cast<result_type>(r);
	}

	// returns pseudo-random number in the range [0, limit)
	template <typename S = float, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
	S GetFloat(S limit)
	{
		S inverse = limit / bound;
		return inverse * GetInt();
	}

	// returns pseudo-random number in the range [0, 1)
	template <typename S = float, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
	S GetFloat()
	{
		return GetFloat<S>(1);
	}

	// returns pseudo-random boolean with given probability of being true
	template <typename S = float, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
	bool GetBool(S probability)
	{
		auto r = GetFloat<S>();
		return probability > r;
	}

	// returns pseudo-random boolean; roughly equivalent to GetBool(.5)
	bool GetBool()
	{
		auto r = GetInt();
		return (r & (1 << (num_bits - 1))) != 0;
	}

	// returns pseudo-random number in the range [0, 1]
	template <typename S = float, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
	S GetFloatInclusive()
	{
		S inverse = S(1) / maximum;
		return inverse * GetInt();
	}

	// returns pseudo-random number in the range [0, max]
	template <typename S = float, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
	S GetFloatInclusive(S max)
	{
		S inverse = max / maximum;
		return inverse * GetInt();
	}

	// returns pseudo-random number in the range [min, max]
	template <typename S = float, typename std::enable_if<std::is_floating_point<S>::value, int>::type dummy = 0>
	S GetFloatInclusive(S min, S max)
	{
		auto range = max - min;
		return min + GetFloatInclusive(range);
	}

	template <typename S = float>
	void GetGaussians(S & y1, S & y2)
	{
		S const one = 1;
		S const two = 2;

		// TODO: Test accuracy of each alternative.
		S x1, x2, w;

		do {
			x1 = two * GetFloatInclusive<S>() - one;
			x2 = two * GetFloatInclusive<S>() - one;
			w = x1 * x1 + x2 * x2;
		} while ( w >= one );

		w = sqrt( (-two * std::log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;

		//// might be faster for non-RISC chipsets
		//S x1 = one - GetFloat<S>();
		//S x2 = GetFloatInclusive<S>();
		//S d = sqrt(-two * std::log(x1));
		//S a = two * static_cast<S>(PI) * x2;
		//y1 = d * cos(a);
		//y2 = d * sin(a);
	}
	
	// constants
	enum
	{
		num_bits = 31,
		bound = generator_type::modulus,
#if defined(_MSC_VER)
		// MSVC implementation of std::minstd_rand::max() is not constexpr
		maximum = generator_type::modulus
#else
		maximum = generator_type::max()
#endif
	};

	// would be nicer if maximum - rather than bound - could be used here;
	// then we'd have same number of even numbers as odd numbers and GetBool would be more accurate
	static_assert((1UL << num_bits) - 1 == bound, "incorrect hand-coded value for Random::num_bits");

	// global seed
	static Random sequence;
	
private:
	////////////////////////////////////////////////////////////////////////////
	// variables

	generator_type generator;
};

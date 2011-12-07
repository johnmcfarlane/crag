/*
 *  Random.h
 *  Crag
 *
 *  Created by john on 5/30/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


// http://www.opengroup.org/onlinepubs/000095399/functions/rand.html
class Random
{
public:
	Random(unsigned long iseed = 1) : seed(iseed) { }
	
	// returns pseudo-random whole number in the range [0, maximum]
	int GetInt()
	{
		seed = seed * 1103515245 + 12345;
		return(static_cast<unsigned>(seed >> (num_bits + 1)) & maximum);
	}
	
	// returns pseudo-random whole number in the range [0, n)
	int GetInt(int n)
	{
		int64_t r = GetInt();
		r *= n;
		r >>= num_bits;
		return static_cast<int>(r);
	}
	
	// returns pseudo-random number in the range [0, 1)
	template <typename S>
	S GetUnit()
	{
		S inverse = S(1) / bound;
		return inverse * GetInt();
	}
	
	// returns pseudo-random number in the range [0, 1]
	template <typename S>
	S GetUnitInclusive()
	{
		S inverse = S(1) / maximum;
		return inverse * GetInt();
	}
	
	template <typename S>
	void GetGaussians(S & y1, S & y2)
	{
		S const one = 1;
		S const two = 2;

		// TODO: Test accuracy of each alternative.
		S x1, x2, w;

		do {
			x1 = two * GetUnitInclusive<S>() - one;
			x2 = two * GetUnitInclusive<S>() - one;
			w = x1 * x1 + x2 * x2;
		} while ( w >= one );

		w = sqrt( (-two * Log( w ) ) / w );
		y1 = x1 * w;
		y2 = x2 * w;

		//// might be faster for RISC chipsets
		//S x1 = one - GetUnit<S>();
		//S x2 = GetUnitInclusive<S>();
		//S d = sqrt(-two * Log(x1));
		//S a = two * static_cast<S>(PI) * x2;
		//y1 = d * Cos(a);
		//y2 = d * Sin(a);
	}
	
	// constants
	enum
	{
		num_bits = 15,
		bound = 1 << num_bits,
		maximum = bound - 1
	};
	
	// global seed
	static Random sequence;
	
private:
	
	unsigned long seed;
};


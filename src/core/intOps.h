//
//  intOps.h
//  crag
//
//  Created by John on 10/26/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

//////////////////////////////////////////////////////////////////////
// Bit-related operations.

// compile-time bit properties

template <unsigned N>
class Number
{
public:
	enum 
	{ 
		BIT_COUNT = Number<N & (N - 1)>::BIT_COUNT + 1,
		FIELD_SIZE = Number<(N >> 1)>::FIELD_SIZE + 1
	};
};

template <>
class Number<0>
{
public:
	enum 
	{ 
		BIT_COUNT = 0,
		FIELD_SIZE = 0
	};
};


// run-time bit properties

template <typename T>
int GetBitCount (T n)  
{
	int bit_count = 0;
	while (n)  
	{
		++ bit_count;
		n &= (n - 1);
	}
	return bit_count;
}

template <typename T>
int GetFieldSize (T n)  
{
	int bit_count = 0;
	while (n)  
	{
		++ bit_count;
		n >>= 1;
	}
	return bit_count;
}


//////////////////////////////////////////////////////////////////////
// TriMod - %3 which assumes a range of [0, 6)

inline int TriMod(int i)
{
	ASSERT(i >= 0 && i < 6);
	
	// Alternative 1
	//	return i % 3;
	
	//	// Alternative 2
	//	return (i < 3) ? i : i - 3;
	
	//	// Alternative 3
	static const int table[6] = { 0, 1, 2, 0, 1, 2 };
	return table[i];
}

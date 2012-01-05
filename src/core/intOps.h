//
//  intOps.h
//  crag
//
//  Created by John on 10/26/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//


//////////////////////////////////////////////////////////////////////
// Bit-related operations.


// run-time bit properties

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

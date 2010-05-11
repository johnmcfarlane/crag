/*
 *  Random.h
 *  Crag
 *
 *  Created by john on 5/30/09.
 *  Copyright 2010 John McFarlane. All rights reserved.
 *
 */

#pragma once


// http://www.opengroup.org/onlinepubs/000095399/functions/rand.html
class Random
{
public:
	Random(unsigned long iseed = 1) : seed(iseed) { }

	// returns pseudo-random whole number in the range [0, MAX]
	int GetInt()
	{
		// TODO: Is this ok in 64 bit?
		seed = seed * 1103515245 + 12345;
		return(static_cast<unsigned>(seed >> 16) & 32767);
	}

	// returns pseudo-random whole number in the range [0, n)
	int GetInt(int n)
	{
		return static_cast<int>((static_cast<int64_t>(GetInt()) * n) / MAX);
		//return static_cast<int>((static_cast<long long>(GetInt()) * n) / MAX);
	}
	
	// returns pseudo-random number in the range [0, 1)
	float GetFloat()
	{
		return static_cast<float>(GetInt()) / (MAX + 1);
	}

	// returns pseudo-random number in the range [0, 1]
	float GetFloatInclusive()
	{
		return static_cast<float>(GetInt()) / MAX;
	}
	
	// returns pseudo-random number in the range [0, 1)
	double GetDouble()
	{
		return static_cast<double>(GetInt()) / (MAX + 1);
	}

	// returns pseudo-random number in the range [0, 1]
	double GetDoubleInclusive()
	{
		return static_cast<double>(GetInt()) / MAX;
	}
	
	void GetGaussians(float & y1, float & y2);
	void GetGaussians(double & y1, double & y2);

	enum 
	{ 
		MAX = 32767 
	};

	static Random sequence;
	
private:

	unsigned long seed;
};


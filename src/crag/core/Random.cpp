/*
 *  Random.cpp
 *  Crag
 *
 *  Created by john on 5/30/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#include "pch.h"

#include "Random.h"
#include "floatOps.h"


Random Random::sequence;	// random

void Random::GetGaussians(float & y1, float & y2)
{
	/*float x1, x2, w;

	do {
		x1 = 2.f * GetFloatInclusive() - 1.f;
		x2 = 2.f * GetFloatInclusive() - 1.f;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1.f );

	w = sqrtf( (-2.f * logf( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;*/
	
	float x1 = 1.f - GetFloat();
	float x2 = GetFloatInclusive();
	float d = sqrtf(-2.f * logf(x1));
	float a = 2.f * static_cast<float>(PI) * x2;
	y1 = d * cosf(a);
	y2 = d * sinf(a);
}

void Random::GetGaussians(double & y1, double & y2)
{
	double x1, x2, w;

	do {
		x1 = 2. * GetFloatInclusive() - 1.;
		x2 = 2. * GetFloatInclusive() - 1.;
		w = x1 * x1 + x2 * x2;
	} while ( w >= 1. );

	w = sqrt( (-2. * log( w ) ) / w );
	y1 = x1 * w;
	y2 = x2 * w;

/*	double x1 = GetDoubleInclusive();
	double x2 = GetDoubleInclusive();
	y1 = sqrt(-2. * log(x1)) * cos(2. * PI * x2);
	y2 = sqrt(-2. * log(x1)) * sin(2. * PI * x2);*/
}

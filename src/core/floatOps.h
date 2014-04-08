//
//  floatOps.h
//  crag
//
//  Created by John on 11/8/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//


#pragma once

#include "core/debug.h"


#if defined(WIN32) && (_M_IX86_FP > 0)
#define __SSE__
#endif


#if defined(__ppc__)
#include <ppc_intrinsics.h>
#endif

#if defined(__SSE__)
#include <xmmintrin.h>
#endif


#define PI 3.14159265358979323846264338327950288419716939937510


//////////////////////////////////////////////////////////////////////
// Basic Math

template<typename T> T Clamped(T const & val, T const & min, T const & max)
{
	ASSERT(min <= max);
	return (val <= max) ? (val >= min) ? val : min : max;
}

template<typename T> void Clamp(T & val, T const & min, T const & max)
{
	ASSERT(min <= max);
	if (val > max)
	{
		val = max;
	}
	else if (val < min)
	{
		val = min;
	}
}

template <typename T>
bool Signbit(T value)
{
#if defined(WIN32)
	return value < 0;
#else
	return std::signbit(value);
#endif
}

template<typename T> T Abs(T const & val)
{
	return std::abs(val);
}

template<typename T> T Diff(T const & a, T const & b)
{
	return Abs(a - b);
}

template<typename T> bool NearEqual(T const & a, T const & b, T const & error)
{
	return Diff(a, b) < error;
}

inline bool NearEqual(float a, float b, float error = .0001f)
{
	return NearEqual<float>(a, b, error);
}

inline bool NearEqual(double a, double b, double error = .0001)
{
	return NearEqual<double>(a, b, error);
}

template<typename T> bool NearEqualLog(T const & a, T const & b, T error)
{
	ASSERT(error > 0);

	T ratio = (a > b) ? (a / b) : (a < b) ? (b / a) : 1;
	return (ratio <= error + 1);
}

template<typename T> T constexpr Squared(T a)
{
	return a * a;
}

template<typename T> T constexpr Cubed(T a)
{
	return a * a * a;
}

template<typename T> T Inverse (T a)
{
	ASSERT(a != 0);
	return static_cast<T>(1) / a;
}

template<typename T> T InvSqrt (T a)
{
	ASSERT(a != 0);
	return Inverse (sqrt (a));
}

#if defined(__ppc__)

inline float FastInvSqrt(float a)
{
	ASSERT(a != 0);
	return __frsqrtes(a);
}

inline double FastInvSqrt(double a)
{
	ASSERT(a != 0);
	return __frsqrte(a);
}

#endif	// defined(__ppc__)

#if defined(__SSE__)

inline float FastInvSqrt ( float f )
{
	ASSERT(f != 0);
	__m128 x = _mm_load_ss( &f );
	float result;
	x = _mm_rsqrt_ss( x ); // x = _mm_rsqrt_ss( x, x ) on some compilers
	_mm_store_ss( &result, x );
	return result;
}

inline double FastInvSqrt (double d)
{
	ASSERT(d != 0);
	float arg = static_cast<float>(d);
	
	_mm_store_ss( & arg, _mm_rsqrt_ss( _mm_load_ss( & arg ) ) );
	
	double  r = arg;
	
	r *= ((3.0 - r * r * d) * 0.5);
	r *= ((3.0 - r * r * d) * 0.5);
	
	return r;
}

#endif	// defined(__SSE__)

#if defined(__arm__)

// This will do the trick at a push.
template<typename T> T FastInvSqrt(T a)
{
	ASSERT(a != 0);
	return Inverse(sqrt(a));
}
#endif	// defined(__arm__))

template <typename T>
inline T CubeRoot(T a)
{
	assert(a >= 0);
#if defined(WIN32)
	return pow(a, T(1. / 3.));
#else
	return cbrt(a);
#endif
}

template<typename T> inline T DegToRad(T d) 
{
	return d * static_cast<T>(PI / 180.0); 
}

template<typename T> inline T RadToDeg(T d) 
{
	return d * static_cast<T>(180.0 / PI); 
}

inline bool IsNaN(double n)
{
#if defined(__GNUC__)
	return std::isnan(n);
#elif defined(WIN32)
	return _isnan(n) != 0;
#endif
}

inline bool IsNaN(float n)
{
	return IsNaN(double(n));
}

inline bool IsInf(double n)
{
#if defined(__GNUC__)
	return std::isinf(n);
#elif defined(WIN32)
	return _finite(n) == 0;
#endif
}

inline bool IsInf(float n)
{
	return IsInf(double(n));
}


//////////////////////////////////////////////////////////////////////
// Bicubic Interpolation
//
// Taken from Paul Breeuwsma: http://www.paulinternet.nl/?page=bicubic

/*template<typename T> T BicubicInterpolation (T p [4][4], T x, T y) 
{
	T a00 = p[1][1];
	T a01 = -.5*p[1][0] + .5*p[1][2];
	T a02 = p[1][0] - 2.5*p[1][1] + 2*p[1][2] - .5*p[1][3];
	T a03 = -.5*p[1][0] + 1.5*p[1][1] - 1.5*p[1][2] + .5*p[1][3];
	T a10 = -.5*p[0][1] + .5*p[2][1];
	T a11 = .25*p[0][0] - .25*p[0][2] - .25*p[2][0] + .25*p[2][2];
	T a12 = -.5*p[0][0] + 1.25*p[0][1] - p[0][2] + .25*p[0][3] + .5*p[2][0] - 1.25*p[2][1] + p[2][2] - .25*p[2][3];
	T a13 = .25*p[0][0] - .75*p[0][1] + .75*p[0][2] - .25*p[0][3] - .25*p[2][0] + .75*p[2][1] - .75*p[2][2] + .25*p[2][3];
	T a20 = p[0][1] - 2.5*p[1][1] + 2*p[2][1] - .5*p[3][1];
	T a21 = -.5*p[0][0] + .5*p[0][2] + 1.25*p[1][0] - 1.25*p[1][2] - p[2][0] + p[2][2] + .25*p[3][0] - .25*p[3][2];
	T a22 = p[0][0] - 2.5*p[0][1] + 2*p[0][2] - .5*p[0][3] - 2.5*p[1][0] + 6.25*p[1][1] - 5*p[1][2] + 1.25*p[1][3] + 2*p[2][0] - 5*p[2][1] + 4*p[2][2] - p[2][3] - .5*p[3][0] + 1.25*p[3][1] - p[3][2] + .25*p[3][3];
	T a23 = -.5*p[0][0] + 1.5*p[0][1] - 1.5*p[0][2] + .5*p[0][3] + 1.25*p[1][0] - 3.75*p[1][1] + 3.75*p[1][2] - 1.25*p[1][3] - p[2][0] + 3*p[2][1] - 3*p[2][2] + p[2][3] + .25*p[3][0] - .75*p[3][1] + .75*p[3][2] - .25*p[3][3];
	T a30 = -.5*p[0][1] + 1.5*p[1][1] - 1.5*p[2][1] + .5*p[3][1];
	T a31 = .25*p[0][0] - .25*p[0][2] - .75*p[1][0] + .75*p[1][2] + .75*p[2][0] - .75*p[2][2] - .25*p[3][0] + .25*p[3][2];
	T a32 = -.5*p[0][0] + 1.25*p[0][1] - p[0][2] + .25*p[0][3] + 1.5*p[1][0] - 3.75*p[1][1] + 3*p[1][2] - .75*p[1][3] - 1.5*p[2][0] + 3.75*p[2][1] - 3*p[2][2] + .75*p[2][3] + .5*p[3][0] - 1.25*p[3][1] + p[3][2] - .25*p[3][3];
	T a33 = .25*p[0][0] - .75*p[0][1] + .75*p[0][2] - .25*p[0][3] - .75*p[1][0] + 2.25*p[1][1] - 2.25*p[1][2] + .75*p[1][3] + .75*p[2][0] - 2.25*p[2][1] + 2.25*p[2][2] - .75*p[2][3] - .25*p[3][0] + .75*p[3][1] - .75*p[3][2] + .25*p[3][3];
	
	T x2 = x * x;
	T x3 = x2 * x;
	T y2 = y * y;
	T y3 = y2 * y;
	
	return a00 + a01 * y + a02 * y2 + a03 * y3 +
	a10 * x + a11 * x * y + a12 * x * y2 + a13 * x * y3 +
	a20 * x2 + a21 * x2 * y + a22 * x2 * y2 + a23 * x2 * y3 +
	a30 * x3 + a31 * x3 * y + a32 * x3 * y2 + a33 * x3 * y3;
}*/


//////////////////////////////////////////////////////////////////////
// [Bi]cubic Interpolation
//
// Taken from GameDev.net: http://www.gamedev.net/community/forums/topic.asp?topic_id=336744
// Still isn't satisfactory.

template<typename T> T CubicInterpolation(T a, T b, T c, T d, T x)
{
	ASSERT(x >= 0 && x <= 1);
	
	T p = (d - c) - (a - b);
	T q = (a - b) - p;
	T r = c - a;
	T s = b;
	T square = x * x;
	
	return (p * (square * x)) + (q * square) + (r * x) + s;
}

template<typename T> T CubicInterpolation(T samples [4], T x)
{
	ASSERT(x >= 0 && x <= 1);
	
	T p = (samples[3] - samples[2]) - (samples[0] - samples[1]);
	T q = (samples[0] - samples[1]) - p;
	T r =  samples[2] - samples[0];
	T s =  samples[1];
	T square = x * x;
	
	return (p * (square * x)) + (q * square) + (r * x) + s;
}

template<typename T> T BicubicInterpolation(T samples [4][4], T x, T y)
{
	return CubicInterpolation(CubicInterpolation(samples[0], x), 
							  CubicInterpolation(samples[1], x), 
							  CubicInterpolation(samples[2], x), 
							  CubicInterpolation(samples[3], x), 
							  y);
}

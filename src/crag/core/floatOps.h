/*
 *  MathUtils.h
 *  Crag
 *
 *  Created by John on 11/8/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


#pragma once

#if ! defined(CRAG_PCH)
#error pch.h not included.
#endif

#include "core/debug.h"

#include <math.h>


#if defined(WIN32) && (_M_IX86_FP > 0)
#define __SSE__
#endif


#if defined(__ppc__)
#include <ppc_intrinsics.h>
#elif defined(__SSE__)
#include <xmmintrin.h>
#else
#error Using intel chipset but __SSE__ not defined. (Hint: GCC uses -msse.)
#endif


#define PI M_PI


//////////////////////////////////////////////////////////////////////
// Simple Math

template<typename T> T Min(T const & a, T const & b)
{
	return (a <= b) ? a : b;
}

template<typename T> T Max(T const & a, T const & b)
{
	return (a >= b) ? a : b;
}

template<typename T> T Clamp(T const & val, T const & min, T const & max)
{
	Assert(min <= max);
	return (val <= max) ? (val >= min) ? val : min : max;
}

template<typename T> T Abs(T const & val)
{
	return (val >= 0) ? val : - val;
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

template<typename T> T Square(T a)
{
	return a * a;
}

template<typename T> T Cube(T a)
{
	return a * a * a;
}

inline float Power(float x, float y)
{
	return powf(x, y);
}

inline double Power(double x, double y)
{
	return pow(x, y);
}

inline float Sqrt(float square)
{
	return sqrtf(square);
}

inline double Sqrt(double square)
{
	return sqrt(square);
}

template<typename T> T Inverse (T a)
{
	return static_cast<T>(1) / a;
}

template<typename T> T InvSqrt (T a)
{
	return Inverse (Sqrt (a));
}

#if defined(__ppc__)

inline float FastInvSqrt(float a)
{
	return __frsqrtes(a);
}

inline double FastInvSqrt(double a)
{
	return __frsqrte(a);
}

#elif defined(__SSE__)

inline float FastInvSqrt ( float f )
{
	__m128 x = _mm_load_ss( &f );
	float result;
	x = _mm_rsqrt_ss( x ); // x = _mm_rsqrt_ss( x, x ) on some compilers
	_mm_store_ss( &result, x );
	return result;
}

inline double FastInvSqrt ( double x )
{
    float arg = static_cast<float>(x);
	
    _mm_store_ss( & arg, _mm_rsqrt_ss( _mm_load_ss( & arg ) ) );
	
    double  r = arg;
    
    r *= ((3.0 - r * r * x) * 0.5);
    r *= ((3.0 - r * r * x) * 0.5);
	
    return r;
}

#else

#error No sneaky inverse square root intrinsics discerned.

// This will do the trick at a push.
template<typename T> T FastInvSqrt(T a)
{
	return Inv(Sqrt(a));
}

#endif

inline float Log(float a) { return logf(a); }
inline double Log(double a) { return log(a); }

inline float Exp(float a) { return expf(a); }
inline double Exp(double a) { return exp(a); }

template<typename T> inline T DegToRad(T d) 
{
	return d * static_cast<T>(PI / 180.0); 
}

template<typename T> inline T RadToDeg(T d) 
{
	return d * static_cast<T>(180.0 / PI); 
}


// Trigonometry
inline float Sin(float a) { return sinf(a); }
inline double Sin(double a) { return sin(a); }

inline float Cos(float a) { return cosf(a); }
inline double Cos(double a) { return cos(a); }

inline float Tan(float a) { return tanf(a); }
inline double Tan(double a) { return tan(a); }

// Trigonometry - inverse
inline float Asin(float x) { return asinf(x); }
inline double Asin(double x) { return asin(x); }

inline float Acos(float x) { return acosf(x); }
inline double Acos(double x) { return acos(x); }

inline float Atan(float x) { return atanf(x); }
inline double Atan(double x) { return atan(x); }
inline float Atan2(float x, float y) { return atan2f(x, y); }
inline double Atan2(double x, double y) { return atan2(x, y); }

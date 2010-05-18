/*
 *  pch.h
 *  Crag
 *
 *  Created by John on 2/8/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */


//////////////////////////////////////////////////////////////////////
// The PCH should be included once at the top of every .cpp file.

#if defined(CRAG_PCH)
#error pch.h included multiple times
#else

#define CRAG_PCH


//////////////////////////////////////////////////////////////////////
// Clean inclusion of <windows.h>.

#if defined(WIN32)

// Set minimum Windows version
#define _WIN32_WINNT _WIN32_WINNT_WIN2K
#include <SDKDDKVer.h>

// Main include for Windows world.
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#undef Yield

// Avoid some annoying warnings.
//#pragma warning(disable : 4355)
#pragma warning(disable : 4530)

#endif


//////////////////////////////////////////////////////////////////////
// Regular includes.

#if defined(WIN32)
#include <SDL.h>
#include <SDL_image.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#endif

#include <stdlib.h>
#include <stdint.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstddef>

#include <limits>
#include <iostream>


//////////////////////////////////////////////////////////////////////
// !0

// c++0x-friendly NULL.
#define nullptr 0

// Semicolon-friendly compound statement,
#define DO(STATEMENT) do { STATEMENT } while (false)

// Semicolon-friendly empty statement.
#define DO_NOTHING do { } while(false)


//////////////////////////////////////////////////////////////////////
// ODE

// ODE is built with either double or single precision.
// Because of the large space which this program simulates,
// double precision is recommended
// which means double-precision ODE
// which means defining dDOUBLE.

#if ! defined(dSINGLE) && ! defined(dDOUBLE)
#define dDOUBLE
#endif


//////////////////////////////////////////////////////////////////////
// Anonymous namespace macro - handy for stubborn debuggers

// Change to anything but "" to get an explicit namespace.
// (Will require appending ANONYMOUS to a lot of vars.)
#define ANONYMOUS

#endif	// CRAG_PCH

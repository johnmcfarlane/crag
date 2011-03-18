/*
 *  atomic.h
 *  crag
 *
 *  Created by John on 10/26/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

// TODO: Replace with SDL_Atomic*

#pragma once

#if defined(__GNUC__)

#define AtomicFetchAndAdd(AUGEND, ADDEND) __sync_fetch_and_add(& AUGEND, ADDEND)
#define AtomicFetchAndSub(AUGEND, ADDEND) __sync_fetch_and_sub(& AUGEND, ADDEND)
#define AtomicMemoryBarrier __sync_synchronize

#elif defined(WIN32)

//#include <intrin.h>
#define AtomicFetchAndAdd(AUGEND, ADDEND) _InterlockedExchangeAdd(& reinterpret_cast<volatile long &>(AUGEND), ADDEND)
#define AtomicMemoryBarrier MemoryBarrier

#endif

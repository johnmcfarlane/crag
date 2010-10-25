/*
 *  atomic.h
 *  crag
 *
 *  Created by John on 10/26/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


#if defined(__GNUC__)
#define AtomicFetchAndAdd(AUGEND, ADDEND) __sync_fetch_and_add(& AUGEND, ADDEND)
#elif defined(WIN32)
#define AtomicFetchAndAdd(AUGEND, ADDEND) _InterlockedExchangeAdd(& reinterpret_cast<volatile long &>(AUGEND), ADDEND)
#endif

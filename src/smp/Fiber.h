//
//  Fiber.h
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include <ucontext.h>

#if defined(__LLP64__) || defined(__LP64__) || defined(WIN64)
// These memory models do not readily support passing of pointers.
// See [http://en.wikipedia.org/wiki/Setcontext#Example].
#define MAKECONTEXT_SMALLER_INT 1
#else
#define MAKECONTEXT_SMALLER_INT 0
#endif

// Note: Fibers are very much NOT a symetric multi-processing feature
// but they are intended to be used in conjunction with the scheduler.
namespace smp
{
	////////////////////////////////////////////////////////////////////////////////
	// Fiber - lightweight thread
	
	class Fiber
    {
        ////////////////////////////////////////////////////////////////////////////////
		// types
	public:
		typedef void (Callback)(void * data);
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Fiber(Callback * callback, void * data, std::size_t stack_size, char const * name);
		~Fiber();
		
		// true iff the caller is running in the Fiber
		bool IsCurrent() const;
		
		// If the given callback has not returned yet.
		bool IsRunning() const;
		
		// Debug/user-friendly identifier.
		char const * GetName() const;
		
		// Continues execution of the fiber.
		void Continue();

		// Pauses execution until the next call to Continue.
		// Must be called from within the fiber.
		void Yield();
		
#if defined(VERIFY)
		void Verify() const;
	private:
		void InitStackUseEstimator();
		std::size_t EstimateStackUse() const;
#endif

		void InitContext();
		void InitCallback(Callback * callback, void * data);

#if MAKECONTEXT_SMALLER_INT
		static void OnLaunchHelper(unsigned i0, unsigned i1, unsigned i2, unsigned i3, unsigned i4, unsigned i5);
#endif
		static void OnLaunch(Fiber & fiber, Callback * callback, void * data);
		
        ////////////////////////////////////////////////////////////////////////////////
		// variables
		std::size_t _stack_size;	// the requested stack size
		char const * _name;	// human-readable name of the Fiber
		bool _is_running;
		ucontext_t _context;
    };
}

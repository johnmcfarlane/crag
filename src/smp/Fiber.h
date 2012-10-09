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
		// constants
		
		// actual stack sizes may vary
		static const std::size_t default_stack_size = 4096;
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Fiber(std::size_t stack_size = default_stack_size);
		~Fiber();
		
#if defined(VERIFY)
		void Verify() const;
#endif
		
		// Continues execution of the fiber.
		void Continue();

		// Creates and launches a new fiber.
		void Launch(Callback * callback, void * data);
		
		// Pauses execution until the next call to Continue.
		// Must be called from within the fiber.
		void Yield();
		
	private:
#if defined(VERIFY)
		void InitStack();
		std::size_t EstimateStackUse() const;
#endif
		
        ////////////////////////////////////////////////////////////////////////////////
		// variables
		ucontext_t _context;
		std::size_t _stack_size;	// the given stack size plus MINSIGSTKSZ
		void * _stack;
    };
}

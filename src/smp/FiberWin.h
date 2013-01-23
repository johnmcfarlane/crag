//
//  Fiber.h
//  crag
//
//  Created by John McFarlane on 2012-03-28.
//  Copyright 2012 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace smp
{
	////////////////////////////////////////////////////////////////////////////////
	// Fiber - lightweight thread
	//
	// This version requires POSIX compatability and is tested under 64-bit linux
	// and /should/ work on OS X with the right preprocessor macros defined.
	
	class Fiber
    {
        ////////////////////////////////////////////////////////////////////////////////
		// types
	public:
		typedef void (Callback)(void * data);
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Fiber(char const * name, std::size_t stack_size, void * data, Callback callback);
		~Fiber();
		
		// must be called by any thread in advance of creating or continuing a fiber
		static void InitializeThread();
		
		// true iff the caller is running in the Fiber
		bool IsCurrent() const;
		
		// If the given callback has not returned yet.
		bool IsRunning() const;
		
		// Debug/user-friendly identifier.
		char const * GetName() const;
		
		// continues execution of the fiber;
		// must be called from outside the fiber
		void Continue();

		// pauses execution until the next call to Continue;
		// must be called from within the fiber
		void Yield();
		
#if defined(VERIFY)
		void Verify() const;
#endif

	private:
		static VOID WINAPI OnLaunch(LPVOID fiber_pointer);
		
        ////////////////////////////////////////////////////////////////////////////////
		// variables
		Callback * _callback;	// given function to call on fiber launch
		void * _data;	// the pointer to pass to _callback
		SIZE_T _stack_size;	// the requested stack size
		SIZE_T _allocated_stack_size;	// amount that was actually requested
		char const * _name;	// human-readable name of the Fiber
		LPVOID _fiber;	// handle to system fiber
		LPVOID _calling_fiber;	// when fiber is running, stores the fiber to which we'll be returning
		bool _is_running;	// true if _callback has not yet exited
    };
}

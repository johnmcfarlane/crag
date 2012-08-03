//
//  Thread.h
//  crag
//
//  Created by John McFarlane on 2011/07/25.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "smp.h"

#include <thread>

namespace smp
{
	// Use Thread to launch and manage a thread. 
	// Specifically, Thread can be used to call a member function from within a newly launched thread.
	// Template parameter, CLASS, is the class and FUNCTION is the member function to launch into.
	// The function parameter, object, is the object for which the member function is called.
	// (Note that two other useful thread-related functions are Sleep and GetNumCpus.)

	class Thread 
	{
		OBJECT_NO_COPY(Thread);
		
		// types
		typedef std::thread::thread ThreadType;
	public:
		typedef void (* Function)(void * data);
		
		// functions
		Thread();
		~Thread();
		
		bool IsLaunched() const;
		
		// True if the calling thread is this thread.
		bool IsCurrent() const;

		// Creates and launches a new thread.
		template <typename FUNCTION_TYPE>
		void Launch(FUNCTION_TYPE const & function)
		{
			_thread = ThreadType([this, function] {
				while (! IsCurrent()) {
					Yield();
				}
				function();
			});
		}
		
		// Waits for thread to return from FUNCTION.
		void Join();

	private:
		template <typename CLASS, Thread CLASS::*THREAD, void (CLASS::*FUNCTION)()>
		static int Callback(void * data)
		{
			ASSERT(data != nullptr);
			CLASS & object = * reinterpret_cast<CLASS *>(data);
			
			// Ensure that the Thread::sdl_thread gets set before progressing.
			// This ensures that IsLaunched returns the correct result.
			Thread const & object_thread = object.*THREAD;
			while (! object_thread.IsLaunched())
			{
				Yield();
			}
			
			// Call the class' member function.
			(object.*FUNCTION)();
			
			return 0;
		}

		ThreadType _thread;
	};
}

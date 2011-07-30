//
//  ObjectThread.h
//  crag
//
//  Created by John McFarlane on 2011/07/25.
//  Copyright 2011 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "Thread.h"


namespace smp
{

	// Use ObjectThread to launch and manage an object-based thread. 
	// Specifically, Thread can be used to call a member function from within a newly launched thread.
	// Template parameter, CLASS, is the class and FUNCTION is the member function to launch into.
	// The function parameter, object, is the object for which the member function is called.
	// (Note that two other useful thread-related functions are Sleep and GetNumCpus.)

	// WARNING: Immediately after launching the new thread it is wise to Sleep that thread. TODO: Is it?
	// WARNING: Otherwise, various of the Thread member functions will return incorrect results.
	// WARNING: This is because the internal state of the class is updated on return from SDL_CreateThread.

	template <typename CLASS> 
	class ObjectThread 
	{
		OBJECT_NO_COPY(ObjectThread);
		
	public:
		ObjectThread()
		{
		}
		
		~ObjectThread()
		{
		}
		
		// Note: The thread may have quit but this will still return true until Kill or Join is called.
		bool IsLaunched() const
		{
			return _thread.IsLaunched();
		}
		
		// True if the calling thread is this thread.
		bool IsCurrent() const
		{
			return _thread.IsCurrent();
		}

		// Creates and launches a new thread which calls object.FUNCTION().
		// If the thread is already running, it is forcefully stopped first.
		// MEMBER points to the member of CLASS which holds this Thread.
		// This is so it can safely update its internal state before progressing.
		template <void (CLASS::*FUNCTION)(), ObjectThread CLASS::*MEMBER>
		void Launch(CLASS & object)
		{
			// Call the given FUNCTION, passing given object, in a new thread. 
			_thread.Launch(Callback<FUNCTION, MEMBER>, reinterpret_cast<void *>(& object));
		}
		
		// Terminates the thread. Risks losing data the thread is working on.
		// If possible, try and use Join instead. 
		void Kill()
		{
			// Shouldn't be called from within the thread.
			_thread.Kill();
		}
		
		// Waits for thread to return from FUNCTION.
		void Join()
		{
			// Shouldn't be called from within the thread.
			_thread.Join();
		}

	private:
		
		template <void (CLASS::*FUNCTION)(), ObjectThread CLASS::*MEMBER>
		static int Callback(void * data)
		{
			Assert(data != nullptr);
			CLASS & object = * reinterpret_cast<CLASS *> (data);
			
			// Ensure that the Thread::sdl_thread gets set before progressing.
			// This ensures 
			ObjectThread const & object_thread = object.*MEMBER;
			while (! object_thread.IsLaunched())
			{
				Sleep(0);
			}
			
			(object.*FUNCTION)();
			return 0;
		}
		
		Thread _thread;
	};
	
}

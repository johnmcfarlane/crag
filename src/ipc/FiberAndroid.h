//
//  FiberAndroid.h
//  crag
//
//  Created by John McFarlane on 2013-04-23.
//  Copyright 2013 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#if defined(__ANDROID__)

#include "smp/Thread.h"

namespace ipc
{
	////////////////////////////////////////////////////////////////////////////////
	// Fiber - lightweight thread
	//
	// This version is used where no POSIX or Windows support is provided.
	
	class Fiber
	{
		OBJECT_NO_COPY(Fiber);

		////////////////////////////////////////////////////////////////////////////////
		// types
	public:
		typedef void (Callback)(void * data);
		
		////////////////////////////////////////////////////////////////////////////////
		// functions
		
		Fiber(char const * name, std::size_t stack_size, void * data, Callback * callback);
		~Fiber();

		// must be called by any thread in advance of creating or continuing a fiber
		static void InitializeThread();
		
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

		static void OnLaunch(Fiber * fiber, void * data, Callback * callback);
		
		////////////////////////////////////////////////////////////////////////////////
		// variables
		char const * _name;	// human-readable name of the Fiber
		smp::Thread _thread;	// hack to deal with lack of ucontext support on Android
		std::condition_variable _condition;	// pauses thread so it can pretend to be a fiber
		std::mutex _condition_mutex;	// mutex used by _condition
		bool _is_running;
		bool _is_yielded;
	};
}

#endif	// defined(__ANDROID__)

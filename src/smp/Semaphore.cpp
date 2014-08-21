//
//  Semaphore.cpp
//  crag
//
//  Created by John on 5/28/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#include "pch.h"

#include "Semaphore.h"

#include "smp.h"

#include "core/app.h"
#include "core/debug.h"

namespace smp 
{ 
	namespace 
	{
		// Version for when SDL Semaphores are working properly.
		// This should hopefully block efficiently.
		class EfficientSemaphore final : public Semaphore
		{
		public:
			EfficientSemaphore(SDL_sem & sdl_semaphore)
			: _sdl_semaphore(sdl_semaphore)
			{
			}
			
			~EfficientSemaphore()
			{
				SDL_DestroySemaphore(& _sdl_semaphore);
			}
			
			ValueType GetValue() const override
			{
				return SDL_SemValue(& _sdl_semaphore);
			}
			
			void Decrement() override
			{
				if (SDL_SemWait(& _sdl_semaphore) != 0)
				{
					DEBUG_BREAK_SDL();
				}
			}
			
			bool TryDecrement() override
			{
				int result = SDL_SemTryWait(& _sdl_semaphore);
				
				switch (result)
				{
					case 0:
						return true;
						
					case SDL_MUTEX_TIMEDOUT:
						return false;
						
					default:
						DEBUG_BREAK_SDL();
						return false;
				}
			}
			
			bool TryDecrement(core::Time timeout) override
			{
				auto milliseconds = Uint32(timeout * 1000);
				int result = SDL_SemWaitTimeout(& _sdl_semaphore, milliseconds);
				
				switch (result)
				{
					case 0:
						return true;
						
					case SDL_MUTEX_TIMEDOUT:
						return false;
						
					default:
						DEBUG_BREAK_SDL();
						return false;
				}
			}
			
			void Increment() override
			{
				if (SDL_SemPost(& _sdl_semaphore) != 0)
				{
					DEBUG_BREAK_SDL();
				}
			}
			
		private:
			// A semaphore - not a SDL_Thread - is necessary for TryLock
			SDL_sem & _sdl_semaphore;
		};


		// Behaves exactly like a semaphore but uses loops for its locks.
		class CompatibleSemaphore final : public Semaphore
		{
		public:
			CompatibleSemaphore(ValueType initial_value)
			: _value(initial_value)
			{
			}
			
			ValueType GetValue() const override
			{
				return _value;
			}
			
			void Decrement() override
			{
				while (! TryDecrement())
				{
					Yield();
				}
			}
			
			bool TryDecrement() override
			{
				ValueType old_value = std::atomic_fetch_sub(& _value, 1);
				if (old_value > 0)
				{
					return true;
				}
				
				std::atomic_fetch_add(& _value, 1);
				return false;
			}
			
			bool TryDecrement(core::Time timeout) override
			{
				auto timeout_time = app::GetTime() + timeout;
				
				while (true)
				{
					if (TryDecrement())
					{
						return true;
					}
					
					if (timeout_time >= app::GetTime())
					{
						return false;
					}
					
					smp::Yield();
				}
			}
			
			void Increment() override
			{
				std::atomic_fetch_add(& _value, 1);
			}
			
		private:
			// A semaphore - not a SDL_Thread - is necessary for TryLock
			std::atomic<ValueType> _value;
		};
	}
	
	// The semaphore makerer.
	Semaphore & Semaphore::Create(ValueType initial_value)
	{
		SDL_sem * sdl_semaphore = SDL_CreateSemaphore(initial_value);
		if (sdl_semaphore != nullptr)
		{
			return * new EfficientSemaphore(* sdl_semaphore);
		}
		
		DEBUG_BREAK("Failed to create Semaphore");
		return * new CompatibleSemaphore(initial_value);
	}
}

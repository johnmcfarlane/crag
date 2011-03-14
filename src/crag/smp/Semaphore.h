/*
 *  Semaphore.h
 *  crag
 *
 *  Created by John on 5/28/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once


namespace smp
{
	class Semaphore
	{
		OBJECT_NO_COPY(Semaphore);
		
	public:
		typedef unsigned int ValueType;
		
		Semaphore(ValueType initial_value = 1);
		~Semaphore();
		
		ValueType GetValue() const;
		
		void Decrement();
		bool TryDecrement();
		void Increment();
		
	private:
		// A semaphore - not a SDL_Thread - is necessary for TryLock
		SDL_sem * sdl_semaphore;
	};
}

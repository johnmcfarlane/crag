//
//  Semaphore.h
//  crag
//
//  Created by John on 5/28/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


namespace smp
{
	class Semaphore
	{
	public:
		// value
		typedef Uint32 ValueType;

		// functions
		OBJECT_NO_COPY(Semaphore);
		
		Semaphore(ValueType initial_value = 0);
		~Semaphore();
		
		ValueType GetValue() const;
		
		void Decrement();
		bool TryDecrement();
		bool TryDecrement(core::Time timeout);
		void Increment();

	private:
		SDL_sem & _sdl_semaphore;
	};
}

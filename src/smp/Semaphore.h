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
		OBJECT_NO_COPY(Semaphore);
		
	protected:
		Semaphore() { }
	public:
		// Should be uint32_t but documentation is full of references to negative values.
		typedef int32_t ValueType;
		
		static Semaphore & Create(ValueType initial_value);
		
		virtual ~Semaphore() { }
		
		virtual ValueType GetValue() const = 0;
		
		virtual void Decrement() = 0;
		virtual bool TryDecrement() = 0;
		virtual void Increment() = 0;
	};
}

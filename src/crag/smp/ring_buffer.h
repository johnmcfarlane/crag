//
//  ring_buffer.h
//  crag
//
//  Created by John McFarlane on 2011/04/21.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "atomic.h"


namespace smp
{
	template <typename T>
	class ring_buffer
	{
	public:

		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef T value_type;
		typedef size_t size_type;
		
		
		////////////////////////////////////////////////////////////////////////////////
		// c'tor/d'tor - obviously NOT thread safe
		
		ring_buffer()
		: first(nullptr)
		, last(nullptr)
		, buffer(nullptr)
		, buffer_end(nullptr)
		{
		}
		
		ring_buffer(size_type c)
		{
			init(c);
		}
		
		~ring_buffer()
		{
			deinit();
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Iterators - NOT thread safe
		
		typedef T * iterator;
		typedef T const * const_iterator;
		
		/*iterator begin()
		{
			return first;
		}
		
		const_iterator begin() const
		{
			return first;
		}
		
		const_iterator cbegin() const
		{
			return first;
		}
		
		iterator end()
		{
			return last;
		}
		
		const_iterator end() const
		{
			return last;
		}
		
		const_iterator cend() const
		{
			return last;
		}*/
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Whole vector-level functions - NOT thread safe
		
		/*void resize(size_type c)
		{
			verify();
			deinit();
			init(c);
		}*/
		
		void clear()
		{
			Assert(false);
			verify();
			
			// Call the d'tor on all the deleted elements.
			/*for (T * it = first; it != last; ++ it)
			{
				it->~T();
			}
			
			last = first;*/
			
			verify();
		}
		
		/*bool empty() const
		{
			verify();
			return first == last;
		}
		
		size_type size() const
		{
			verify();
			return last - first;
		}
		
		size_type capacity() const
		{
			verify();
			return everything - first;
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Element access functions - NOT thread safe
		
		T & operator [] (size_type index)
		{
			Assert(index < size());
			return first [index];
		}
		
		T const & operator [] (size_type index) const
		{
			Assert(index < size());
			return first [index];
		}
		
		T & front() const
		{
			Assert(first != 0);
			return * first;
		}
		
		T & back() const
		{
			Assert(last > first);
			return * (last - 1);
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Element append functions - thread safe
		// These are atomic fetch and push back operations.
		
		void push_back(T const & value)
		{
			grow(1) = value;
		}
		
		T & grow(size_type num)
		{
			// Increase the array.
			T * pre_last = grow_uninit (num);
			
			// Initialize the new elements.
			T const * post_last = pre_last + num;
			for (T * it = pre_last; it != post_last; ++ it)
			{
				new (it) T;
			}
			
			// Return the first of the new elements.
			return * pre_last;
		}
		
		T & grow(size_type num, T & filler)
		{
			// Increase the array.
			T * pre_last = grow_uninit (num);
			
			// Initialize the new elements.
			T const * post_last = pre_last + num;
			for (T * it = pre_last; it != post_last; ++ it)
			{
				new (it) T (filler);
			}
			
			// Return the first of the new elements.
			return * pre_last;
		}
		
	private:
		
		// Grows the vector but doesn't initialize the newcomers.
		T * grow_uninit(size_type num)
		{
			typedef int atomic_type;
            static_assert(sizeof(void *) == sizeof(atomic_type), "cannot cast pointer to int for fetch-add atomic");
			atomic_type & last_bytes = reinterpret_cast<atomic_type &> (last);
			atomic_type increment_bytes = sizeof(T) * num;
			atomic_type fetched_bytes = AtomicFetchAndAdd (last_bytes, increment_bytes);
			return reinterpret_cast<T *> (fetched_bytes);
		}*/
		
		void init(size_type c) 
		{
			// Calculate the size of the required memory buffer in bytes,
			size_type c_bytes = sizeof(T) * c;
			
			// Allocate the buffer and set first and last to the start of it.
			char * c_buffer = reinterpret_cast<char *> (Allocate (c_bytes, CalculateAlignment(sizeof(T))));
			Assert(c_buffer != nullptr);
			
			// set attributes
			first = last = buffer = reinterpret_cast<T *> (c_buffer);
			buffer_end = buffer + c;
			
			verify();
		}
		
		void deinit()
		{
			verify();
			
			// Erase all the allocated elements.
			clear();
			
			// Free up the memory.
			Free(buffer);
		}
		
		T * wrap(T * p)
		{
			if (p >= buffer_end)
			{
				p -= buffer_end;
				Assert(p < buffer_end);
			}
			return p;
		}
		
		void verify() const
		{
			if (first == nullptr)
			{
				Assert(last == nullptr);
				Assert(buffer == nullptr);
				Assert(buffer_end == nullptr);
			}
			else
			{
				verify_ptr(first);
				verify_ptr(last);
				verify_ptr(buffer_end);
			}
		}
		
		void verify_ptr(const T * p)
		{
			Assert(buffer + (p - buffer) == p);
			Assert(p >= buffer && p < buffer_end);
		}
		
		T * first;
		T * last;
		T * buffer;
		T * buffer_end;
	};
	
}

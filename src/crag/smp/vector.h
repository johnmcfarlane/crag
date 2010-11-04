/*
 *  vector.h
 *  crag
 *
 *  Created by John on 10/23/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "atomic.h"


namespace smp
{
	
	// This is a very limited subset of std::vector which is intended for 
	// writing by multiple threads simultaneously without need for locks. 
	// Note that operators such as push_back are useless as 
	// the new value of back() could change before it is called. 
	template <typename T>
	class vector
	{
	public:
		
		////////////////////////////////////////////////////////////////////////////////
		// types
		
		typedef T value_type;
		typedef size_t size_type;

		
		////////////////////////////////////////////////////////////////////////////////
		// c'tor/d'tor - obviously NOT thread safe
		
		vector()
		: first(nullptr)
		, last(nullptr)
		, everything(nullptr)
		{
		}
		
		vector(size_type c)
		: first(nullptr)
		, last(nullptr)
		, everything(nullptr)
		{
			init(c);
		}
		
		~vector()
		{
			deinit();
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Iterators - NOT thread safe
		
		typedef T * iterator;
		typedef T const * const_iterator;
		
		iterator begin()
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
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Whole vector-level functions - NOT thread safe
		
		void reserve(size_type c)
		{
			verify();
			deinit();
			init(c);
		}
		
		void clear()
		{
			verify();

			// Call the d'tor on all the deleted elements.
			for (T * it = first; it != last; ++ it)
			{
				it->~T();
			}
			
			last = first;
			
			verify();
		}
		
		bool empty() const
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
			size_type & last_bytes = reinterpret_cast<size_type &> (last);
			size_type increment_bytes = sizeof(T) * num;
			size_type fetched_bytes = AtomicFetchAndAdd (last_bytes, increment_bytes);
			return reinterpret_cast<T *> (fetched_bytes);
		}
		
		void init(size_type c) 
		{
			// Calculate the size of the required memory buffer in bytes,
			size_type c_bytes = sizeof(T) * c;
			
			// Allocate the buffer and set first and last to the start of it.
			char * buffer = reinterpret_cast<char *> (Allocate (c_bytes, CalculateAlignment(sizeof(T))));
			first = last = reinterpret_cast<T *> (buffer);
			
			// Set everything to end of buffer.
			buffer += c_bytes;
			everything = reinterpret_cast<T *> (buffer);

			verify();
		}
		
		void deinit()
		{
			verify();

			// Erase all the allocated elements.
			clear();
			
			// Free up the memory.
			Free(first);
		}
		
		void verify() const
		{
			// Either all members are null or non-null; no mix and match.
			Assert((first == nullptr) == (last == nullptr));
			Assert((first == nullptr) == (everything == nullptr));
			
			// First, last and everything are in ascending order.
			Assert(first <= last);
			Assert(last <= everything);
			
			// Points are correctly lined up and add up.
			Assert(first + (last - first) == last);
			Assert(first + (everything - first) == everything);
		}

		T * first;
		T * last;
		T * everything;
	};
	
}

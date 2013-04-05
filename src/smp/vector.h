//
//  vector.h
//  crag
//
//  Created by John on 10/23/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once


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
		
		typedef value_type * ptr_type;
		typedef uintptr_t atomic_type;
		typedef std::atomic<atomic_type> atomic;
		
		////////////////////////////////////////////////////////////////////////////////
		// c'tor/d'tor - obviously NOT thread safe
		
		vector()
		: first(nullptr)
		, last(0)
		, everything(nullptr)
		{
		}
		
		vector(size_type c)
		{
			init(c);
		}
		
		~vector()
		{
			deinit();
		}
				
		////////////////////////////////////////////////////////////////////////////////
		// invariant verification
		
#if defined(VERIFY)
		void Verify() const
		{
			// Either all members are null or non-null; no mix and match.
			VerifyEqual((begin() == nullptr), (end() == nullptr));
			VerifyEqual((begin() == nullptr), (everything == nullptr));
			
			// First, last and everything are in ascending order.
			VerifyOp(begin(), <=, end());
			VerifyOp(end(), <=, everything);
			
			// Points are correctly lined up and add up.
			VerifyEqual(begin() + (end() - begin()), end());
			VerifyEqual(begin() + (everything - begin()), everything);
		}
#endif
		
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
			atomic_type last_value = last;
			ptr_type last_ptr = reinterpret_cast<ptr_type>(last_value);
			return last_ptr;
		}
		
		const_iterator end() const
		{
			atomic_type last_value = last;
			ptr_type last_ptr = reinterpret_cast<ptr_type>(last_value);
			return last_ptr;
		}
		
		const_iterator cend() const
		{
			return iterator(last);
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Whole vector-level functions - NOT thread safe
		
		void reserve(size_type c)
		{
			VerifyObject(* this);
			deinit();
			init(c);
		}
		
		void clear()
		{
			VerifyObject(* this);

			// Call the d'tor on all the deleted elements.
			for (T& element : *this)
			{
				element.~T();
			}
			
			last = reinterpret_cast<atomic_type>(first);
			
			VerifyObject(* this);
		}
		
		bool empty() const
		{
			VerifyObject(* this);
			return first == last;
		}
		
		size_type size() const
		{
			VerifyObject(* this);
			return size_type(end() - begin());
		}
		
		size_type capacity() const
		{
			VerifyObject(* this);
			return everything - first;
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Element access functions - NOT thread safe
		
		T & operator [] (size_type index)
		{
			ASSERT(index < size());
			return first [index];
		}
		
		T const & operator [] (size_type index) const
		{
			ASSERT(index < size());
			return first [index];
		}
		
		T & front() const
		{
			ASSERT(first != 0);
			return * first;
		}
		
		T & back() const
		{
			ASSERT(last > first);
			return * (end() - 1);
		}
		
		
		////////////////////////////////////////////////////////////////////////////////
		// Element append functions - thread safe
		// These are atomic fetch and push back operations.
		
		T & push_back(T const & value)
		{
			return * grow(1, value);
		}
		
		iterator grow(size_type num)
		{
			// Increase the array.
			T * pre_last = grow_uninit (num);
			
			// Initialize the new elements.
			T const * post_last = pre_last + num;
			for (T * it = pre_last; it != post_last; ++ it)
			{
				new (it) T;
			}
			
			VerifyObject(* this);

			// Return the first of the new elements.
			return pre_last;
		}
		
		iterator grow(size_type num, T const & filler)
		{
			// Increase the array.
			T * pre_last = grow_uninit (num);
			
			// Initialize the new elements.
			T const * post_last = pre_last + num;
			for (T * it = pre_last; it != post_last; ++ it)
			{
				new (it) T (filler);
			}
			
			VerifyObject(* this);

			// Return the first of the new elements.
			return pre_last;
		}
		
	private:
		
		// Grows the vector but doesn't initialize the newcomers.
		T * grow_uninit(size_type num)
		{
			ASSERT(std::atomic_is_lock_free(& last));
			uintptr_t addend_byte = sizeof(value_type) * num;
			atomic_type fetched_bytes = last.fetch_add(static_cast<atomic_type>(addend_byte));
			return reinterpret_cast<T *> (fetched_bytes);
		}
		
		void init(size_type c) 
		{
			// Allocate the buffer and set first and last to the start of it.
			first = Allocate<T>(c);
			last = reinterpret_cast<atomic_type> (first);
			
			// Set everything to end of buffer.
			everything = first + c;

			VerifyObject(* this);
		}
		
		void deinit()
		{
			VerifyObject(* this);

			// Erase all the allocated elements.
			clear();
			
			// Free up the memory.
			Free(first);
		}

		////////////////////////////////////////////////////////////////////////////////
		// variables

		ptr_type first;
		atomic last;
		ptr_type everything;
	};
}

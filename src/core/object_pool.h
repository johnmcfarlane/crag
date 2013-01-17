//
//  object_pool.h
//  crag
//
//  created by john on 4/28/09.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

#include "core/debug.h"

//////////////////////////////////////////////////////////////////////
// Object pool macros

#define DEFINE_OBJECT_POOL(TYPE, MIN_OBJECTS) \
	static core::object_pool<TYPE> pool(MIN_OBJECTS); \
	void* TYPE::operator new(size_t sz) \
	{ \
		ASSERT(sz == sizeof(TYPE)); \
		return pool.allocate(); \
	} \
	void* TYPE::operator new [](size_t sz) \
	{ \
		ASSERT(sz == sizeof(TYPE)); \
		DEBUG_BREAK("not supported"); \
		return nullptr; \
	} \
	void TYPE::operator delete(void* p) \
	{ \
		pool.free(p); \
	} \
	void TYPE::operator delete [](void* p) \
	{ \
		DEBUG_BREAK("not supported"); \
	}

#define DECLARE_OBJECT_POOL(TYPE) \
	void* operator new(size_t sz); \
	void* operator new [](size_t sz); \
	void operator delete(void* p); \
	void operator delete [](void* p);

namespace core
{
	// A simple memory allocation pool which allocates/deallocates very quickly
	// but can become unsorted quickly
	template<typename T> class object_pool
	{
		////////////////////////////////////////////////////////////////////////
		// types

		struct Node
		{
			Node * next;
		};
		static_assert(sizeof(T) >= sizeof(Node), "size of value_type must be greater than that of a pointer");

	public:
		typedef size_t size_type;
		typedef T value_type;

		////////////////////////////////////////////////////////////////////////
		// functions

		// c/d'tor
		object_pool() 
		{
			init();
		}

		object_pool(size_type max_elements) 
		{
			init(max_elements);
		}
	
		~object_pool()
		{
			deinit();
		}
	
		value_type * begin()
		{
			return _array;
		}
	
		value_type const * begin() const
		{
			return _array;
		}
	
		value_type * end()
		{
			return _array_end;
		}
	
		value_type const * end() const
		{
			return _array_end;
		}
	
		void sort_free()
		{
			ASSERT(empty());

			init_free_list();
		}
		
		// returns uninitialized block of memory large enough to object of type, value_type
		// or nullptr if the pool is full
		void * allocate()
		{
			VerifyObject(* this);

			if (_free_list == nullptr)
			{
				DEBUG_MESSAGE("pool is full");
				return nullptr;
			}

			auto ptr = reinterpret_cast<void *>(_free_list);
			_free_list = _free_list->next;
			
			VerifyElement(reinterpret_cast<value_type *>(ptr));
			++ _num_allocated;

			return ptr;
		}
	
		// frees up memory returned from allocate
		void free(void * ptr)
		{
			VerifyObject(* this);

			if (ptr == nullptr)
			{
				return;
			}

			VerifyElement(reinterpret_cast<value_type *>(ptr));

			Node * n = reinterpret_cast<Node *>(ptr);
		
			// Add back into free list.
			n->next = _free_list;
			_free_list = n;

			-- _num_allocated;
		}

		// returns a new object constructed with given parameters
		// or nullptr if the pool is full
		template <typename ... PARAMETERS>
		value_type * create(PARAMETERS && ... parameters)
		{
			VerifyObject(* this);

			value_type * object = reinterpret_cast<value_type *>(allocate());
			if (object != nullptr)
			{
				// c'tor
				new (object) value_type (std::forward<PARAMETERS ...>(parameters) ...);
			}

			return object;
		}
	
		// deletes an object returned by create
		void destroy(value_type * ptr)
		{
			VerifyObject(* this);

			if (ptr == nullptr)
			{
				return;
			}

			// d'tor
			ptr->~value_type();

			// Add back into free list.
			free(ptr);
		}

		// returns the maximum count of objects
		size_type capacity() const
		{
			return _array_end - _array;
		}
	
		// returns the current number of allocated objects
		size_type count() const
		{
			return _num_allocated;
		}
	
		bool empty() const
		{
			return count() == 0;
		}

		void VerifyElement(value_type const * element) const
		{
#if defined(VERIFY)
			VerifyRef(* element);
			VerifyArrayElement(element, begin(), end());
#endif
		}

#if defined(VERIFY)
		void Verify() const
		{
			auto const loop_limit = 10u;

			auto free_list_size = 0u;
			for (auto node_iterator = _free_list; free_list_size < loop_limit && node_iterator != nullptr; node_iterator = node_iterator->next)
			{
				VerifyElement(reinterpret_cast<value_type *>(node_iterator));
				++ free_list_size;
			}

			auto num_free = std::min(capacity() - _num_allocated, loop_limit);
			VerifyEqual(num_free, free_list_size);
		}
#endif
	
	private:
		// init/deinit
		void init()
		{
			_array = nullptr;
			_array_end = nullptr;
			_free_list = nullptr;
			_num_allocated = 0;
		}
	
		void init(size_type max_num_elements)
		{
			ASSERT(max_num_elements != 0);

			_array = Allocate<value_type>(max_num_elements);
			_array_end = _array + max_num_elements;
			init_free_list();
			_num_allocated = 0;
			
			//Verify();
		}
	
		void init_free_list()
		{
			_free_list = reinterpret_cast<Node *>(_array);
		
			for (value_type * it = _array; ; )
			{
				Node * & node_next = reinterpret_cast<Node *>(it)->next;
				++ it;
			
				if (it == _array_end)
				{
					node_next = 0;	// Last element has a null next.
					break;
				}
			
				node_next = reinterpret_cast<Node *>(it);
			}
		}
	
		void deinit()
		{
			ASSERT(_num_allocated == 0);

			Free(_array);
		}

		////////////////////////////////////////////////////////////////////////
		// variables

		// array start/end
		value_type * _array;
		value_type * _array_end;

		// free List
		Node * _free_list;
		size_type _num_allocated;	// number allocated in the pool
	};
}

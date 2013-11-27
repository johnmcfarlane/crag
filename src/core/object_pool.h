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

// define this if you want everything to run slower
//#define CRAG_CORE_OBJECT_POOL_DIAGNOSTICS

// fixed-size pool allocation;
// placed with class member definitions
// and add DECLARE_ALLOCATOR to the class definition
#define DEFINE_POOL_ALLOCATOR(TYPE, MIN_OBJECTS) \
	static core::object_pool<TYPE> pool(MIN_OBJECTS); \
	void * TYPE::operator new(size_t sz) noexcept \
	{ \
		if (sz != sizeof(TYPE)) \
		{ \
			DEBUG_BREAK("allocator for object of type " #TYPE " and size " SIZE_T_FORMAT_SPEC "B asked to allocate " SIZE_T_FORMAT_SPEC "B.", sizeof(TYPE), sz); \
		} \
		void * ptr = pool.allocate(); \
		if (ptr == nullptr) \
		{ \
			DEBUG_BREAK("failed to allocate object of type " #TYPE); \
		} \
		return ptr; \
	} \
	void * TYPE::operator new [] (size_t) noexcept \
	{ \
		DEBUG_BREAK("not supported"); \
		return nullptr; \
	} \
	void TYPE::operator delete(void * p) noexcept \
	{ \
		pool.free(p); \
	} \
	void TYPE::operator delete [](void *) noexcept \
	{ \
		DEBUG_BREAK("not supported"); \
	}

namespace core
{
	// A simple memory allocation pool which allocates/deallocates very quickly
	// but can become unsorted quickly
	template <typename T>
	class object_pool
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
		object_pool(size_type max_elements) 
		{
			init(max_elements);

			CRAG_VERIFY(* this);
		}
	
		~object_pool()
		{
			CRAG_VERIFY(* this);

			deinit();
		}

		// tl;dr: don't call this function!
		// calls function on the range of elements which have been allocated since the 
		// last free list reset; is useful because it includes the set of all currently
		// allocated elements; care must be taken to not corrupt elements which are not
		// currently allocated; it is non-trivial to tell which those are; debug-only
		// checks are in place to catch this
		template <typename Function>
		void for_each_activated(Function function)
		{
#if defined(NDEBUG)
			std::for_each(_array, reinterpret_cast<value_type *>(_unlinked_begin), function);
#else
			std::for_each(_array, reinterpret_cast<value_type *>(_unlinked_begin), [function] (value_type & element)
			{
				Node const & node = reinterpret_cast<Node const &>(element);
				Node const * next = node.next;

				function(element);

				// did the given function corrupt a deallocated element?
				ASSERT(next == node.next);
			});
#endif
		}
		
		// returns uninitialized block of memory large enough to hold object of type, value_type
		// or nullptr if the pool is full
		void * allocate()
		{
			CRAG_VERIFY(* this);

			auto new_list_element = reinterpret_cast<value_type *>(_free_list_head);

			if (_free_list_head == _unlinked_begin)
			{
				if (new_list_element == _array_end)
				{
					DEBUG_MESSAGE("object pool is full");
					return nullptr;
				}

				_free_list_head = reinterpret_cast<Node *>(new_list_element + 1);
				_unlinked_begin = _free_list_head;
			}
			else
			{
				_free_list_head = _free_list_head->next;
			}
			
			++ _num_allocated;
			
			CRAG_VERIFY(* this);
			return reinterpret_cast<void *>(new_list_element);
		}
	
		// frees up memory returned from allocate
		void free(void * ptr)
		{
			CRAG_VERIFY(* this);

			if (ptr == nullptr)
			{
				return;
			}

			CRAG_VERIFY_ARRAY_ELEMENT(reinterpret_cast<value_type *>(ptr), _array, reinterpret_cast<value_type const *>(_unlinked_begin));

			Node * n = reinterpret_cast<Node *>(ptr);
		
			// Add back into free list.
			n->next = _free_list_head;
			_free_list_head = n;

			-- _num_allocated;
			if (_num_allocated == 0)
			{
				init_free_list();
			}

			CRAG_VERIFY(* this);
		}

		// returns a new object constructed with given parameters
		// or nullptr if the pool is full
		template <typename ... PARAMETERS>
		value_type * create(PARAMETERS && ... parameters)
		{
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
		
		bool empty() const
		{
			return _num_allocated == 0;
		}

#if defined(CRAG_VERIFY_ENABLED)
		// element must be any member of the array
		void VerifyElement(value_type const & element) const
		{
			CRAG_VERIFY_ARRAY_ELEMENT(& element, _array, _array_end);
		}

		// element must be any member of the array which is currently allocated
		void VerifyAllocatedElement(value_type const & element) const
		{
			CRAG_VERIFY_ARRAY_ELEMENT(& element, _array, reinterpret_cast<value_type const *>(_unlinked_begin));
		}

#if defined(CRAG_VERIFY_ENABLED)
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_BEGIN(object_pool, pool)
			CRAG_VERIFY_ARRAY_ELEMENT(reinterpret_cast<value_type *>(pool._free_list_head), pool._array, pool._array_end);
			CRAG_VERIFY_ARRAY_POINTER(reinterpret_cast<value_type *>(pool._unlinked_begin), reinterpret_cast<value_type *>(pool._free_list_head), pool._array_end);
			CRAG_VERIFY_ARRAY_POINTER(pool._array_end, reinterpret_cast<value_type *>(pool._unlinked_begin), pool._array_end);
			CRAG_VERIFY_OP(pool._array_end, >, pool._array);

#if defined(CRAG_CORE_OBJECT_POOL_DIAGNOSTICS)
			std::size_t free_list_size = 0;
			for (auto node_iterator = pool._free_list_head; node_iterator != pool._unlinked_begin; node_iterator = node_iterator->next)
			{
				VerifyFreeElement(* reinterpret_cast<value_type *>(node_iterator));
				++ free_list_size;
			}

			std::size_t max_free_list_size = reinterpret_cast<value_type *>(pool._unlinked_begin) - pool._array;
			CRAG_VERIFY_OP(free_list_size, <=, max_free_list_size);
			CRAG_VERIFY_EQUAL(_num_allocated, max_free_list_size - free_list_size);
#endif
		CRAG_VERIFY_INVARIANTS_DEFINE_TEMPLATE_END
#endif	// defined(CRAG_VERIFY_ENABLED)

		// element must be any member of the array which is currently not allocated
		void VerifyFreeElement(value_type const & element) const
		{
			VerifyElement(element);

			auto & node = reinterpret_cast<Node const &>(element);
			auto next = reinterpret_cast<value_type const *>(node.next);
			CRAG_VERIFY_ARRAY_POINTER(next, _array, reinterpret_cast<value_type const *>(_unlinked_begin));
		}
#endif

	private:
		// init/deinit
		void init(size_type max_num_elements)
		{
			ASSERT(max_num_elements != 0);

#if defined(NDEBUG)
			// round capacity up to nearest page size
			size_t required_num_bytes = RoundToPageSize(sizeof(value_type) * max_num_elements);
			max_num_elements = required_num_bytes / sizeof(value_type);
#endif

			_array = reinterpret_cast<value_type*>(AllocatePage(get_allocation_size(max_num_elements)));
			_array_end = _array + max_num_elements;
			init_free_list();
			_num_allocated = 0;
		}
		
		void init_free_list()
		{
			_free_list_head = reinterpret_cast<Node *>(_array);
			_unlinked_begin = reinterpret_cast<Node *>(_array);
		}
	
		void deinit()
		{
			if (_num_allocated != 0)
			{
				DEBUG_MESSAGE("object pool destroyed with " SIZE_T_FORMAT_SPEC " object remaining", _num_allocated);
			}

			FreePage(_array, get_allocation_size(capacity()));
		}

		size_t get_allocation_size(size_t capacity) const
		{
			return RoundToPageSize(sizeof(value_type) * capacity);
		}

		////////////////////////////////////////////////////////////////////////
		// variables

		// array start/end
		value_type * _array;
		value_type * _array_end;

		// free List
		Node * _free_list_head;
		
		// after last element in the free/used sets
		Node * _unlinked_begin;
		
		size_type _num_allocated;
	};
}

/*
 *  Pool.h
 *  Crag
 *
 *  Created by john on 4/28/09.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "core/debug.h"


template<class T> class Pool
{
	struct Node
	{
		Node * next;
	};

public:
	Pool() 
	{
		Init();
	}

	Pool(int max_elements) 
	{
		Init(max_elements);
	}
	
	~Pool()
	{
		Deinit();
	}
	
	void Resize(int max_num_elements)
	{
		if (array + max_num_elements != end)
		{
			Deinit();
			Init(max_num_elements);
		}
	}
	
	void Clear()
	{
		Deinit();
		Init();
	}
	
	T * Alloc()
	{
		VerifyObjectPtr(this);
		if (free_list != nullptr)
		{
			T * element = reinterpret_cast<T *>(free_list);
			free_list = free_list->next;

			Assert(IsValidElement(element));

			if (element >= end_used)
			{
				end_used = element + 1;
			}
			
			return element;
		}
		
		return nullptr;
	}
	
	void Free(T * ptr)
	{
		Assert (ptr != nullptr);

#if VERIFY
		T const * element = reinterpret_cast<T *>(ptr);

		// Check this is a valid element pointer for this pool. 
		Assert(IsValidElement(element));
		
		// Check this isn't a double-free.
///////////////////////////////////		Assert(! IsFree(element));
#endif

		Node * n = reinterpret_cast<Node *>(ptr);
		
		// Add back into free list.
		n->next = free_list;
		free_list = n;
	}
	
	// Element Access
	T & operator [] (int i)
	{
		Assert(i >= 0);
		Assert(i < GetCapacity());
		return array[i];
	}
	
	T const & operator [] (int i) const
	{
		Assert(i >= 0);
		Assert(i < GetCapacity());
		return array[i];
	}
	
	int GetIndex(T const & element) const
	{
		Assert(& element >= array);
		Assert(& element < end);
		return & element - array;
	}
	
	T const * GetArray() const
	{
		return array;
	}
	
	T * GetArray()
	{
		return array;
	}
	
	int GetCapacity() const
	{
		return end - array;
	}
	
	int GetMaxUsed() const
	{
		return end_used - array;
	}
	
	// Slow!
	int GetNumFree() const
	{
		return GetListSize(free_list);
	}
	
	int GetNumUsed() const
	{
		return GetCapacity() - GetNumFree();
	}
	
	// Slow!
	bool IsEmpty() const
	{
		return GetCapacity() == GetNumFree();
	}

	bool IsValidElement(T const * element) const
	{
		if (element < array)
		{
			return false;
		}
		
		if (element >= end)
		{
			return false;
		}
		
		//VerifyTrue (array + GetIndex(* element) == element);
		return true;
	}
	
#if VERIFY
	void Verify() const
	{
		VerifyTrue (free_list == nullptr || IsValidElement(reinterpret_cast<T *>(free_list)));
	}
#endif
	
#if ! defined(NDEBUG) 
	bool IsFree(T const & element) const
	{
		Node const * node = reinterpret_cast<Node const *>(& element);
	
		// Heuristic: if it's a valid-looking node, then assume it's ok.
		Node const * next = node->next;
		if (next == nullptr)
		{
			// Oh dear. Are we at the end of the list?
			// Unlikely, so lets do the slow check - just in case.
			return IsFree_Slow(reinterpret_cast<T const *>(node));
		}
		
		// Is it a valid Element reference? 
		return IsValidElement(reinterpret_cast<T const *>(next));
	}
	
	bool IsFree_Slow(T const * element) const
	{
		Node const * node = reinterpret_cast<Node const *>(element);
	
		for (Node const * free_node = free_list; free_node != 0; free_node = free_node->next)
		{
			if (free_node == node)
			{
				return true;
			}
		}
		
		return false;
	}
#endif

private:

	// Init/Deinit
	void Init()
	{
		array = 0;
		end = 0;
		end_used = 0;
		free_list = 0;
		//VerifyTrue(this);
	}
	
	void Init(int max_num_elements)
	{
		if (max_num_elements != 0)
		{
			Assert(max_num_elements > 0);
			
			int buffer_size_bytes = max_num_elements * sizeof(T);
			array = reinterpret_cast<T *>(new char [buffer_size_bytes]);
			end = array + max_num_elements;
			end_used = array;
			free_list = reinterpret_cast<Node *>(array);
		
			for (T * it = array; ; )
			{
				Node * & node_next = reinterpret_cast<Node *>(it)->next;
				++ it;

				if (it == end)
				{
					node_next = 0;	// Last element has a null next.
					break;
				}
				
				node_next = reinterpret_cast<Node *>(it);
			}
		}
		else
		{
			Init();
		}
		
		//Verify();
	}
	
	void Deinit()
	{
		//Verify();
		delete [] reinterpret_cast<char *>(array);
	}
	
	// Free List
	int GetListSize(Node const * node) const
	{
		int list_size = 0;
		while (node != 0)
		{
			//Assert (IsValidElement(* reinterpret_cast<T const *>(node)));
			node = node->next;
			++ list_size;
		}
		return list_size;
	}
	
	// Element Attributes
	T * array;
	T const * end;
	T const * end_used;	// Remembers the highest array element allocated.

	// Free List
	Node * free_list;
};



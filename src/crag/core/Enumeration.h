/*
 *  Enumeration.h
 *  crag
 *
 *  Created by John McFarlane on 11/11/10.
 *  Copyright 2009, 2010 John McFarlane. All rights reserved.
 *  This program is distributed under the terms of the GNU General Public License.
 *
 */

#pragma once

#include "Singleton.h"


namespace core
{
	
	template <typename VT>
	bool operator < (VT const & lhs, VT const & rhs)
	{
		return strcmp(lhs.GetName(), rhs.GetName()) < 0;
	}
	
	
	////////////////////////////////////////////////////////////////////////////////
	// Enumeration class
	// 
	// Stores an alphabetized, global list of all objects of type, ELEMENT_TYPE.
	// Optimized to store lists of globally scoped objects.
	// Thus, manages to avoid using dynamic allocation.
	// ELEMENT_TYPE must be derived from Enumeration<ELEMENT_TYPE>::node.
	// Used for development-related odds and ends such as config values and profiling stats.
	
	template <typename ELEMENT_TYPE> 
	class Enumeration : public Singleton < Enumeration<ELEMENT_TYPE> >
	{
	public:
		typedef char const * name_type;
		typedef ELEMENT_TYPE value_type;

		// c'tor
		Enumeration()
		{
			sort();
		}
		
		
		// minimal stl-style iterator for scanning through the list
		class iterator
		{
		public:
			
			iterator() : _ptr (nullptr) { }
			iterator(value_type * & n) : _ptr (& n) { }
			
			value_type * & operator * ()
			{
				return * _ptr;
			}
			
			value_type * & operator -> ()
			{
				return * _ptr;
			}
			
			friend bool operator == (iterator const & lhs, iterator const & rhs)
			{
				return * lhs._ptr == * rhs._ptr;
			}
			
			friend bool operator != (iterator const & lhs, iterator const & rhs)
			{
				return * lhs._ptr != * rhs._ptr;
			}
			
			iterator & operator ++ () 
			{
				_ptr = & (* _ptr)->_next;
				return * this;
			}
			
			name_type GetName () const
			{
				Verify ();
				Assert (* _ptr != nullptr);
				return (* _ptr)->_name;
			}
			
		private:
			
			void Verify() const
			{
				Assert(_ptr != nullptr);
			}
			
			value_type * * _ptr;
		};
		
		
		// the base class for the objects to be stored in the Enumeration
		// Derive from this the class of objects that are to be enumerated.
		class node
		{
			OBJECT_NO_COPY(node);
			
			friend class iterator;
			friend class Enumeration;
			
		public:
			
			node(name_type name)
			: _name(name)
			{
				// up-cast 
				value_type * value = static_cast<value_type *>(this);
				
				// Insert this into list.
				_next = _head;
				_head = value;
				
				// Basically, make sure there's no multiple inheritance going on.
				// TODO: Make this compile-time.
				Assert(_head == this);
			}
			
			virtual ~node()
			{
			}
			
			name_type GetName() const
			{
				return _name;
			}
			
		private:
			
			name_type _name;
			value_type * _next;
		};
		
		
		// accessors
		
		static iterator begin ()
		{
			return iterator (_head);
		}
		
		static iterator end ()
		{
			return iterator (_tail);
		}
		
		static iterator find (name_type name)
		{
			iterator i = begin();
			
			while (i != end())
			{
				int c = strcmp(name, i.GetName());
				if (c <= 0)
				{
					if (c < 0)
					{
						return end();
					}
					else
					{
						return i;
					}
				}
				
				++ i;
			}
			
			return i;
		}
		
	private:
		
		// Sort the contents of the list.
		static void sort()
		{
			// Move the main (unsorted) list into a temporary list.
			value_type * temp_head = _head;
			
			// Empty the main list.
			_head = nullptr;
			
			// While the temp list is not empty,
			while (temp_head != nullptr)
			{
				// remove the head of the temp list,
				value_type & inserted = * temp_head;
				temp_head = inserted._next;
				
				// and for all elements in the main list,
				for (iterator i = begin(); ; ++ i)
				{
					// skip if they're in the correct order relative to the element to be inserted.
					if (i != end())
					{
						value_type & insertion_point = * * i;
						if (insertion_point < inserted)
						{
							continue;
						}
					}
					
					// Otherwise, here is where the inserted element should be inserted.
					inserted._next = * i;
					(* i) = & inserted;
					break;
				}
			}
		}
		
		static value_type * _head;
		static value_type * _tail;	// never changes; needed for end()
	};
	
	
	template <typename ELEMENT_TYPE> ELEMENT_TYPE * Enumeration<ELEMENT_TYPE>::_head = nullptr;
	template <typename ELEMENT_TYPE> ELEMENT_TYPE * Enumeration<ELEMENT_TYPE>::_tail = nullptr;
	
}

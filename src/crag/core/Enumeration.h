/*
 *  enumeration.h
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

		
		// minimal stl-style iterator for scanning through the list
		// TODO: Make protected and create const_iterator.
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
		class node
		{
			OBJECT_NO_COPY(node);
			
			friend class iterator;
			
		public:
			
			node(name_type name)
			: _name(name)
			{
				// Find the insertion point.
				iterator match = Enumeration::find_nearest(name);
				
				// Make sure it's not a perfect match, i.e. a duplicate.
				Assert(* match == nullptr || compare_name(match->GetName(), name) != 0);
				
				// Insert this into list.
				_next = * match;
				* match = static_cast<value_type *>(this);
				
				// Basically, make sure there's no multiple inheritance going on.
				// TODO: Make this compile-time.
				Assert(* match == this);
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

		
		// interface
		
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
				int c = compare_name(name, i.GetName());
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
		
		// returns iterator of the element directly preceeding or matching the given name
		static iterator find_insertion (name_type name)
		{
			iterator i = begin();
			
			while (i != end())
			{
				int c = compare_name(name, i.GetName());
				if (c <= 0)
				{
					return i;
				}
				
				++ i;
			}
			
			return i;
		}
		
	private:
		static int compare_name(name_type lhs, name_type rhs)
		{
			return strcmp(lhs, rhs);
		}

		static int compare_node(node const & lhs, node const & rhs)
		{
			return compare_name(lhs.GetName(), rhs.GetName());
		}

		static value_type * _head;
		static value_type * _tail;	// never changes; needed for end()
	};
	
	
	template <typename ELEMENT_TYPE> ELEMENT_TYPE * Enumeration<ELEMENT_TYPE>::_head = nullptr;
	template <typename ELEMENT_TYPE> ELEMENT_TYPE * Enumeration<ELEMENT_TYPE>::_tail = nullptr;
	
}

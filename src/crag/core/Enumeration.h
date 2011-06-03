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
#include "intrusive_list.h"


namespace core
{
	
	template <typename TYPE>
	class Enumeration
	{
		OBJECT_NO_COPY(Enumeration);
		
		typedef TYPE value_type;
		
		typedef char const * name_type;
		name_type _name;
		
		typedef intrusive::hook<Enumeration> hook_type;
		hook_type _hook;

		typedef intrusive::list<Enumeration, & Enumeration::_hook> list_type;
		static list_type _values;
	public:
		typedef typename list_type::const_iterator const_iterator;
		typedef typename list_type::iterator iterator;
		
		Enumeration(name_type name)
		: _name(name)
		{
			// note: asserts may fail horribly before main begins.
			Assert(find(_name) == nullptr);	// entries must be unique
			Assert(static_cast<value_type *>(this) == this);	// this must be base for TYPE 
			
			iterator insertion = std::upper_bound(begin(), end(), _name, compare);
			_values.insert(insertion, * this);
			
			Assert(find(_name) != nullptr);
		}
		
		~Enumeration()
		{
			_values.remove(* this);
		}
		
		bool operator==(name_type name) const
		{
			return (name == _name) || (strcmp(name, _name) == 0);
		}
		
		operator value_type & ()
		{
			return * static_cast<value_type *>(this);
		}
		
		operator value_type const & () const
		{
			return * static_cast<value_type const *>(this);
		}
		
		name_type GetName() const
		{
			return _name;
		}
		
		static value_type * find(name_type name)
		{
			iterator found = std::find(begin(), end(), name);
			if (found == end())
			{
				return nullptr;
			}
			else
			{
				Enumeration & found_enumeration = * found;
				value_type & found_value = found_enumeration;
				return & found_value;
			}
		}
		
		static const_iterator begin()
		{
			return _values.begin();
		}
		
		static const_iterator end()
		{
			return _values.end();
		}
		
	private:
		static bool compare(name_type lhs, Enumeration const & rhs)
		{
			return strcmp(lhs, rhs._name) < 0;
		}
	};
	
	
	template <typename TYPE> 
	typename Enumeration<TYPE>::list_type Enumeration<TYPE>::_values;
}

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


#define ENUMERATED_CLASS(CLASS) \
public: \
	friend class core::Enumeration<CLASS>; \
	typedef core::Enumeration<CLASS> Enumeration; \
	typedef intrusive::hook hook_type; \
	typedef char const * name_type; \
	void InitEnumeration(name_type name) { Enumeration::add(* this); _name = name; } \
	char const * GetName() const { return _name; } \
private: \
	hook_type _hook; \
	name_type _name


namespace core
{
	
	template <typename ELEMENT>
	class Enumeration
	{
	protected:
		typedef ELEMENT value_type;
		typedef intrusive::list<value_type, & value_type::_hook> list;
	public:
		typedef typename list::iterator iterator;
		typedef typename value_type::name_type name_type;
		
		static void add(ELEMENT & addition)
		{
			_values.push_back(addition);
		}
		
		static value_type * find(name_type name)
		{
			find_by_name f(name);
			iterator i = std::find_if(_values.begin(), _values.end(), f);
			if (i == _values.end())
			{
				return nullptr;
			}
			
			value_type & value = * i;
			return & value;
		}
		
		static iterator begin()
		{
			return _values.begin();
		}
		
		static iterator end()
		{
			return _values.end();
		}
		
		static bool sort_function(name_type const & lhs, name_type const & rhs)
		{
			return strcmp(lhs, rhs);
		}
		static bool sort_function(value_type const & lhs, value_type const & rhs)
		{
			return sort_function(lhs._name, rhs._name);
		}
		class find_by_name
		{
		public:
			find_by_name(name_type name) : _name(name) { }
			bool operator()(value_type const & value) const
			{
				// I guess the linker failed to whittle down the strings.
				// Use the slower strcmp test instead.
				Assert((strcmp(value._name, _name) == 0) == (value._name == _name));
				
				return value._name == _name;
			}
		private:
			name_type _name;
		};
		
	private:
		static list _values;
	};
	
	
	template <typename ELEMENT> typename Enumeration<ELEMENT>::list Enumeration<ELEMENT>::_values;
	
}

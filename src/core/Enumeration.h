//
//  Enumeration.h
//  crag
//
//  Created by John McFarlane on 11/11/10.
//  Copyright 2009, 2010 John McFarlane. All rights reserved.
//  This program is distributed under the terms of the GNU General Public License.
//

#pragma once

namespace core
{
	
	template <typename TYPE>
	class Enumeration
	{
		OBJECT_NO_COPY(Enumeration);
		
		typedef TYPE value_type;
		
		typedef char const *key_type;
		key_type _key;
		
		typedef intrusive::hook<Enumeration> hook_type;
		hook_type _hook;

		DEFINE_TEMPLATED_INTRUSIVE_LIST_TYPE(Enumeration, _hook, list_type);
	public:
		typedef typename list_type::const_iterator const_iterator;
		typedef typename list_type::iterator iterator;
		
		Enumeration(key_type key)
		: _key(key)
		{
			// note: asserts may fail horribly before main begins.
			ASSERT(find(_key) == nullptr);	// entries must be unique
			ASSERT(static_cast<value_type *>(this) == this);	// this must be base for TYPE 
			
			iterator insertion = std::upper_bound(begin(), end(), * this);
			get_values().insert(insertion, * this);
			
			ASSERT(find(_key) != nullptr);
		}
		
		~Enumeration()
		{
			get_values().remove(* this);
		}

		bool operator<(Enumeration const & rhs) const
		{
			return (_key != rhs._key) && strcmp(_key, rhs._key) < 0;
		}
		
		bool operator==(key_type key) const
		{
			return (key == _key) || (strcmp(key, _key) == 0);
		}
		
		operator value_type & ()
		{
			return * static_cast<value_type *>(this);
		}
		
		operator value_type const & () const
		{
			return * static_cast<value_type const *>(this);
		}
		
		key_type GetKey() const
		{
			return _key;
		}
		
		static value_type * find(key_type key)
		{
			iterator found = std::find(begin(), end(), key);
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
		
		static iterator begin()
		{
			return get_values().begin();
		}
		
		static iterator end()
		{
			return get_values().end();
		}

	private:
		static list_type & get_values()
		{
			// the global list of objects of this type
			static list_type values;
			return values;
		}
	};
}
